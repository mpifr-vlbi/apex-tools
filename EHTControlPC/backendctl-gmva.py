#!/usr/bin/python
# Issue commands to interact with VLBI backend devices or subsets of it.
# Subsets can be any of the following:
#   - all devices beloning to the whole backend
#   - all devices belonging to one or more chains (i.e. data channels)
#   - one or more devices of a certain type
#
# TODO:
# - add -v verbose flag
# - add -n dry-run flag
# - add -h help flag
# - test suite that verifies that for all commands in the __doc__ the
#   required list functions and exec targets exist.
'''
Usage: backendctl [OPTIONS] [SUBSET] { COMMAND | help }

OPTIONS
  -c --configfile=FILE  Use non-standard config file
  -n --dry-run          Do not execute command but print it to the screen
  -v --verbose          Display debugging information

SUBSET
  whole     All backend devices [default]
  chain     Devices involved in chain (a.k.a. data channel)
  bdc       block-down converter
  r2dbe     Roach2 digitizer
  dbbc3     dbbc3
  mark6     Mark6 recorder
  cc        control computer

COMMAND (for the default subset 'whole')
  graph         Plot the backend configuration in a graph
  schedule      Show/Start a schedule defined by a vexfile
  configure     Init devices (if necessary) and apply config file
  check         Run tests to verify proper setup
  reconfigure   Reinit devices and apply config file (READ help FIRST)
'''

CONFIGFILE_FILENAME = '/etc/backend-gmva.conf'
ACTIONFILE_FILENAME = 'schedactions.conf'
BECS_RELATIVE_PATH = 'becs/bin'
DRYRUN = 0
VERBOSE = 0

# Define all possible input commands.
# The purpose of this list is to give a clear overview of the
# input command structure.
cmd_doc = """
whole .                 # All backend devices
whole show .            # Show backend properties
whole show graph        # Plot the backend configuration in a graph
whole show graph-format FORMAT # Plot the backend configuration in a graph
whole configure         # Init devices (if necessary) and apply config file
whole reconfigure       # Reinit devices and apply config file (READ help FIRST)
whole check             # Run tests to verify proper setup

whole schedule .        # Show/Start a schedule defined by a vexfile
whole schedule follow trigger   # Perform actions timed to the scans defined in the triggered schedule
whole schedule follow TESTVEX   # Perform actions timed to the scans defined in a vexfile from the testing area
whole schedule make DELAY NSCAN SCANLEN GAPLEN  # Create a test schedule in the testing area

chain ID .              # Devices involved in chain (a.k.a. data channel)
chain ID configure      # Init devices (if necessary) and apply config file
chain ID reconfigure    # Reinit devices and apply config file (READ help FIRST)
chain ID check CHECK    # Run tests to verify proper setup

bdc ID .                # Block-down converters
bdc ID configure        # Init devices (if necessary) and apply config file
bdc ID reconfigure      # Reinit devices and apply config file (READ help FIRST)
bdc ID check            # Run tests to verify proper setup
bdc ID get .            # Read current values for device parameters
bdc ID get attn         # Read current attenuator level
bdc ID get attn OUTPORT # Read current attenuator level
bdc ID set .            # Set values for device parameters
bdc ID set attn VALUE   # Set attenuator level
bdc ID set attn OUTPORT VALUE   # Set attenuator level
bdc ID set band BAND    # Select filter band
bdc ID set ctrl MODE    # Enable touch pad and remote control
bdc ID reset .          # Reset initial values for device parameters
bdc ID reset ctrl       # Remove control locks


r2dbe ID .              # Roach2 digitizers
r2dbe ID configure      # Init devices (if necessary) and apply config file
r2dbe ID reconfigure    # Reinit devices and apply config file (READ help FIRST)
r2dbe ID check          # Run tests to verify proper setup
r2dbe ID run .          # Start an activity, such as a calibration process
r2dbe ID run 2bit-cal INPORT    # Calibrate 2-bit threshold
r2dbe ID run adc-qc-cal INPORT  # Calibrate ADC quad-cores

mark6 ID .              # Mark6 recorders
mark6 ID configure      # Init devices (if necessary) and apply config file
mark6 ID reconfigure    # Reinit devices and apply config file (READ help FIRST)
mark6 ID check          # Run tests to verify proper setup
mark6 ID schedule load trigger  # Copy vexfile and start M6_CC process on device
mark6 ID schedule load TESTVEX  # Copy vexfile and start M6_CC process on device
mark6 ID schedule unload        # Stop any running M6_CC processes and abort any pending / ongoing recordings
mark6 ID run test-recording DELAY SCANLEN       # Record a short scan on given list of devices and check results
mark6 ID modules SLOT . # Mark6 disk modules
mark6 ID modules SLOT init      # Initialize modules
mark6 ID modules SLOT init-fresh        # Wipe modules and initialize
mark6 ID modules SLOT init-fresh MSN    # Wipe modules and initialize and set Module Serial Number
mark6 ID group unmount  # Unmount group of modules

dbbc3 ID .              # Dbbc3 devices
dbbc3 ID configure      # Init devices (if necessary) and apply config file
dbbc3 ID reconfigure    # Reinit devices and apply config file (READ help FIRST)
dbbc3 ID check          # Run tests to verify proper setup

cc .                    # The control computer
cc check                # Run tests to verify proper setup
"""

import os
backendctl_home = os.path.dirname(os.path.realpath(__file__))

import config
fname = os.path.join(backendctl_home, CONFIGFILE_FILENAME)
cfg = config.Config(fname)

#========================
#-- leaf-node definitions
#========================
# exec prepare function takes kwargs created by the parser and derives new ones from them.
# e.g. ID + PORT -> PORTF
# e.g. ID -> DEVID
from collections import namedtuple
import docparse



def kwarg_id2dev(devtype):
    if devtype == 'chain':
        def was_chain(kwargs):
            devs = cfg.list_chains_with_devices()
            selectors = kwargs['ID'].split(',')
            devids = {d: None for c,ds in devs.items() for d in ds if c in selectors}
            kwargs['DEVID'] = ','.join(devids)
            return kwargs
        return was_chain
    else:
        def was_dev(kwargs):
            ports = cfg.list_devids_with_outports(devtype)
            selectors = kwargs['ID'].split(',')
            devids = {d.split(cfg.delim_port)[0]: None for c,ds in ports.items() for d in ds if c in selectors}
            kwargs['DEVID'] = ','.join(devids)
            return kwargs
        return was_dev

def kwarg_id2chain(kwargs):
    kwargs['CHAINID'] = kwargs['ID']
    if 'all' in kwargs['ID']: kwargs['CHAINID'] = ','.join(cfg.chains)
    return kwargs

def kwarg_subselect_ports(devtype):
    def f(kwargs):
        ports = cfg.list_devids_with_outports(devtype)
        po, pi = [], []
        for d in kwargs['ID'].split(','):
            po.extend(ports[d])
        for p in po:
            pi.append(cfg.outport_to_inport(p))
        #-- insert matches
        for k in ['OUTPORT', 'INPORT']:
            if k == 'OUTPORT': pavail = po
            elif k == 'INPORT': pavail = pi
            if not k in kwargs: continue
            plong = {}
            ppsel = kwargs[k].split(',')
            #-- all ports are selected
            if 'all' in ppsel:
                kwargs[k+'F'] = ','.join(list(pavail))
                continue
            #-- a list of ports is selected
            for p in pavail:
                pp = p.split(cfg.delim_port)[1]
                if pp in ppsel:
                    plong[p] = None
            kwargs[k+'F'] = ','.join(list(plong))
        return kwargs
    return f

class IncompatibleArgumentLengths(docparse.DocparseException): pass
def kwarg_zip(kwa, kwb):
    def f(kwargs):
        x, y = kwargs[kwa].split(','), kwargs[kwb].split(',')
        if len(x) != len(y): raise IncompatibleArgumentLengths('{}({}) != {}({})'.format(len(x), kwa, len(y), kwb))
        k = '%s_%s'%(kwa, kwb)
        kwargs[k] = ','.join(['%s=%s'%(a, b) for a,b in zip(x, y)])
        return kwargs
    return f

# Define output command lines for all possible input lines.
# Each input line is named after its path.  The path is a
# underscore-separated sequence of node names.  Nodes names
# are a combination of the respective command
# name and the number of arguments that belong to the node.
LeafDefs = namedtuple('LeafnodeDefinitions', 'exec prepare')
leafnode_defs = {
    'test0_a2': LeafDefs('test0_a2', lambda x: x),
    'test1_b1': LeafDefs('test1_b1', lambda x: x),
    'test2_c0': LeafDefs('test2_c0', lambda x: x),
    'whole0_show0_graph0':      LeafDefs('{backendctl_home:}/graph-config.py {configfile:} pdf', lambda x: x),
    'whole0_show0_graph-format1':       LeafDefs('{backendctl_home:}/graph-config.py {configfile:} {FORMAT:}',       lambda x: x),
    'whole0_configure0':        LeafDefs('{becsbin:}/all_conf.py --config-file {configfile:} {DEVID:}',     lambda x: kwarg_id2dev('chain')({**x, 'ID': 'all'})),
    'whole0_reconfigure0':      LeafDefs('{becsbin:}/all_conf.py --config-file {configfile:} --force-reconfig {DEVID:}',        lambda x: kwarg_id2dev('chain')({**x, 'ID': 'all'})),
    'whole0_check0':            LeafDefs('{becsbin:}/all_check.py --config-file {configfile:} {DEVID:}',    lambda x: kwarg_id2dev('chain')({**x, 'ID': 'all'})),
    'whole0_schedule0_follow0_trigger0':        LeafDefs('{becsbin:}/run_sched.py --config-file {configfile:} --action-file {actionfile:}',     lambda x: x),#kwarg_id2dev('chain')({**x, 'ID': 'all'})),
    'whole0_schedule0_follow1': LeafDefs('{becsbin:}/run_sched.py --test-vex {TESTVEX:} --config-file {configfile:} --action-file {actionfile:}',     lambda x: x),#kwarg_id2dev('chain')({**x, 'ID': 'all'})),
    'whole0_schedule0_make4':   LeafDefs('{becsbin:}/test_sched.py --config-file {configfile:} --sched-param {DELAY:} {NSCAN:} {SCANLEN:} {GAPLEN:}',  lambda x: kwarg_id2dev('chain')({**x, 'ID': 'all'})),
    'chain1_configure0':        LeafDefs('{becsbin:}/all_conf.py --config-file {configfile:} {DEVID:}',     kwarg_id2dev('chain')),
    'chain1_reconfigure0':      LeafDefs('{becsbin:}/all_conf.py --config-file {configfile:} --force-reconfig {DEVID:}',        kwarg_id2dev('chain')),
    'chain1_check1':            LeafDefs('{becsbin:}/chain_check.py --config-file {configfile:} --chk-type {CHECK:} {CHAINID:}',        kwarg_id2chain),
    'bdc1_set0_attn1':          LeafDefs('{becsbin:}/bdc_set_attn.py --config-file {configfile:} {OUTPORTF_VALUE:}',    lambda x: kwarg_zip('OUTPORTF', 'VALUE')(kwarg_subselect_ports('bdc')({**x, 'OUTPORT': 'all'}))),
    'bdc1_set0_attn2':          LeafDefs('{becsbin:}/bdc_set_attn.py --config-file {configfile:} {OUTPORTF_VALUE:}',    lambda x: kwarg_zip('OUTPORTF', 'VALUE')(kwarg_subselect_ports('bdc')(x))),
    'bdc1_get0_attn0':          LeafDefs('{becsbin:}/bdc_get_attn.py --config-file {configfile:} {OUTPORTF:}',          lambda x: kwarg_subselect_ports('bdc')({**x, 'OUTPORT': 'all'})),
    'bdc1_get0_attn1':          LeafDefs('{becsbin:}/bdc_get_attn.py --config-file {configfile:} {OUTPORTF:}',          kwarg_subselect_ports('bdc')),
    'bdc1_set0_band1':          LeafDefs('{becsbin:}/bdc_set_band.py --config-file {configfile:} {BAND:} {DEVID:}',     kwarg_id2dev('bdc')),
    'bdc1_set0_ctrl1':          LeafDefs('{becsbin:}/bdc_set_ctrl.py --config-file {configfile:} {MODE:} {DEVID:}',     kwarg_id2dev('bdc')),
    'bdc1_reset0_ctrl0':        LeafDefs('{becsbin:}/bdc_set_ctrl.py --config-file {configfile:} --clear both {DEVID:}',        kwarg_id2dev('bdc')),
    'bdc1_configure0':          LeafDefs('{becsbin:}/all_conf.py --config-file {configfile:} {DEVID:}',     kwarg_id2dev('bdc')),
    'bdc1_reconfigure0':        LeafDefs('{becsbin:}/all_conf.py --config-file {configfile:} --force-reconfig {DEVID:}',        kwarg_id2dev('bdc')),
    'bdc1_check0':              LeafDefs('{becsbin:}/all_check.py --config-file {configfile:} {DEVID:}',    kwarg_id2dev('bdc')),
    'r2dbe1_run0_2bit-cal1':    LeafDefs('{becsbin:}/r2dbe_cal.py --config-file {configfile:} --cal-type 2bit {INPORTF:}',   kwarg_subselect_ports('r2dbe')),
    'r2dbe1_run0_adc-qc-cal1':  LeafDefs('{becsbin:}/r2dbe_cal.py --config-file {configfile:} --cal-type qcore {INPORTF:}',  kwarg_subselect_ports('r2dbe')),
    'r2dbe1_configure0':        LeafDefs('{becsbin:}/all_conf.py --config-file {configfile:} {DEVID:}',     kwarg_id2dev('r2dbe')),
    'r2dbe1_reconfigure0':      LeafDefs('{becsbin:}/all_conf.py --config-file {configfile:} --force-reconfig {DEVID:}',        kwarg_id2dev('r2dbe')),
    'r2dbe1_check0':            LeafDefs('{becsbin:}/all_check.py --config-file {configfile:} {DEVID:}',    kwarg_id2dev('r2dbe')),
    'dbbc31_configure0':        LeafDefs('{becsbin:}/all_conf.py --config-file {configfile:} --chain-id {ID:} --force-reconfig {DEVID:}',     kwarg_id2dev('dbbc3')),
    'dbbc31_reconfigure0':      LeafDefs('{becsbin:}/all_conf.py --config-file {configfile:} --chain-id {ID:} --force-reconfig {DEVID:}',        kwarg_id2dev('dbbc3')),
    'dbbc31_check0':            LeafDefs('{becsbin:}/all_check.py --config-file {configfile:} --chain-id {ID:} {DEVID:} ',    kwarg_id2dev('dbbc3')),
    'mark61_modules1_init0':    LeafDefs('{becsbin:}/mark6_mod_init.py --config-file {configfile:} {DEVID:} {SLOT:}',   kwarg_id2dev('mark6')),
    'mark61_modules1_init-fresh0':      LeafDefs('{becsbin:}/mark6_mod_init.py --new --config-file {configfile:} {DEVID:} {SLOT:}',     kwarg_id2dev('mark6')),
    'mark61_modules1_init-fresh1':      LeafDefs('{becsbin:}/mark6_mod_init.py --new --config-file {configfile:} {DEVID:} {SLOT_MSN:}', lambda x: kwarg_zip('SLOT', 'MSN')(kwarg_id2dev('mark6')(x))),
    'mark61_group0_unmount0':   LeafDefs('{becsbin:}/mark6_group_cmd.py --config-file {configfile:} --cmd-type unmount {DEVID:}',       kwarg_id2dev('mark6')),
    'mark61_schedule0_load0_trigger0':  LeafDefs('{becsbin:}/mark6_load_sched.py --config-file {configfile:} {DEVID:}',         kwarg_id2dev('mark6')),
    'mark61_schedule0_load1':   LeafDefs('{becsbin:}/mark6_load_sched.py --config-file {configfile:} --test-vex {TESTVEX:} {DEVID:}',         kwarg_id2dev('mark6')),
    'mark61_schedule0_unload0':   LeafDefs('{becsbin:}/mark6_unload_sched.py --config-file {configfile:} {DEVID:}',         kwarg_id2dev('mark6')),
    'mark61_run0_test-recording2':      LeafDefs('{becsbin:}/mark6_test_record.py --config-file {configfile:} {DEVID:} --delay {DELAY:} --duration {SCANLEN:}', kwarg_id2dev('mark6')),
    'mark61_configure0':        LeafDefs('{becsbin:}/all_conf.py --config-file {configfile:} {DEVID:}',     kwarg_id2dev('mark6')),
    'mark61_reconfigure0':      LeafDefs('{becsbin:}/all_conf.py --config-file {configfile:} --force-reconfig {DEVID:}',        kwarg_id2dev('mark6')),
    'mark61_check0':            LeafDefs('{becsbin:}/all_check.py --config-file {configfile:} {DEVID:}',    kwarg_id2dev('mark6')),
    'cc0_check0':               LeafDefs('{becsbin:}/cc_check.py --config-file {configfile:}',  lambda x: x),
    }

#==========================
#-- select-node definitions
#==========================
from functools import reduce
from contextlib import suppress

# Define listing functions for each node that has one or more arguments.
# Nodes are identified by path since node names themselves need not be
# unique, e.g. whole0_check1 vs bdc1_check1.
#
# Each function maps n input values x_n to m output values, where n is
# the number of input arguments to upstream nodes and m is the number
# of variables the current node has.
# Example:
# node2_node2_node3: f(x) 4 -> 3
#
# The result for each node variable (each one of 3 in the example above)
# is a list with possible values for that variable, or 'type'.  'type'
# acts as a wildcard to match anything string that can be cast to that
# particular type.  An empty list matches nothing.
def list_dwo(devtype):
    dwo = cfg.list_devids_with_outports(devtype)
    return {k: ['all', *{e.split(cfg.delim_port)[1]: None for e in v}] for k,v in dwo.items()}
def list_dwi(devtype):
    dwi = cfg.list_devids_with_inports(devtype)
    return {k: ['all', *{e.split(cfg.delim_port)[1]: None for e in v}] for k,v in dwi.items()}
def list_testable_vexfiles():
    fnames = []
    with suppress(FileNotFoundError):
        fnames = os.listdir('/srv/vexstore/testing/')
    fnames = [f for f in fnames if f[-4:] == '.vex']
    #TODO: drop expired vexfiles
    return fnames
def is_msn(msn):
    '''MSN is string of 8 non-whitespace characters'''
    return len(msn) == 8  and  not ' ' in msn
def is_attn(value):
    '''Value is integer multiple of 0.5 in range [0, 31.5]'''
    try: fval = float(value)
    except ValueError: return False
    if fval < 0 or fval > 31.5: return False
    return int(fval*2) == fval*2
def isfloat(value):
    '''Value is castable to type float'''
    try: fval = float(value)
    except ValueError: return False
    return True
def isint(value):
    '''Value is castable to type int'''
    try: int(value)
    except ValueError: return False
    return True
SelectDefs = namedtuple('SelectnodeDefinitions', ('list', 'count'))
selectnode_defs = {
    'test0_a2':         SelectDefs(lambda *x: (['b'], ['c']), lambda *x: [1, 1]),
    'test1':            SelectDefs(lambda *x: (['a'], ), lambda *x: [1]),
    'test1_b1':         SelectDefs(lambda *x: (['c'], ), lambda *x: [1]),
    'test2':            SelectDefs(lambda *x: (['a'], ['b']), lambda *x: [1, 1]),
    'whole0_show0_graph-format1':       SelectDefs(lambda *x: (['pdf', 'gv', 'svg', 'png', 'jpg', 'bmp', 'dot'], ), lambda *x: [1]),
    'whole0_schedule0_follow1':         SelectDefs(lambda *x: (list_testable_vexfiles(), ), lambda *x: [1]),
    'whole0_schedule0_make4':           SelectDefs(lambda *x: (isint, isint, isint, isint), lambda *x: [1, 1, 1, 1]),
    'chain1':           SelectDefs(lambda *x: ([*cfg.chains, 'all'], ), lambda *x: [None]),
    'chain1_check1':    SelectDefs(lambda *x: (['routing', 'timesync'], ), lambda *x: [1]),
    'bdc1':             SelectDefs(lambda *x: (list(list_dwo('bdc')), ), lambda *x: [None]),
    'bdc1_get0_attn1':  SelectDefs(lambda *x: (reduce(lambda a, e: a + list_dwo('bdc')[e], x[0].split(','), []), ), lambda *x: [None]),
    'bdc1_set0_attn1':  SelectDefs(lambda *x: (is_attn, ),
        lambda *x: [len((kwarg_subselect_ports('bdc')({'ID': x[0], 'OUTPORT': 'all'})['OUTPORTF']).split(','))]),
    'bdc1_set0_attn2':  SelectDefs(lambda *x: (reduce(lambda a, e: a + list_dwo('bdc')[e], [x[0]], []), is_attn),
        lambda *x: [None, len((kwarg_subselect_ports('bdc')({'ID': x[0], 'OUTPORT': x[1]})['OUTPORTF']).split(','))]),
    'bdc1_set0_band1':  SelectDefs(lambda *x: (['4to8', '5to9'], ), lambda *x: [1]),
    'bdc1_set0_ctrl1':  SelectDefs(lambda *x: (['remote', 'both'], ), lambda *x: [1]),
    'r2dbe1':           SelectDefs(lambda *x: (list(list_dwo('r2dbe')), ), lambda *x: [None]),
    'r2dbe1_run0_2bit-cal1':    SelectDefs(lambda *x: (reduce(lambda a, e: a + list_dwi('r2dbe')[e], x[0].split(','), []), ), lambda *x: [None]),
    'r2dbe1_run0_adc-qc-cal1':  SelectDefs(lambda *x: (reduce(lambda a, e: a + list_dwi('r2dbe')[e], x[0].split(','), []), ), lambda *x: [None]),
    'dbbc31':           SelectDefs(lambda *x: (list(list_dwo('dbbc3')), ), lambda *x: [None]),
    'mark61':           SelectDefs(lambda *x: (list(list_dwo('mark6')), ), lambda *x: [None]),
    'mark61_modules1':  SelectDefs(lambda *x: (['1', '2', '3', '4'], ), lambda *x: [1]),
    'mark61_modules1_init-fresh1':  SelectDefs(lambda *x: (is_msn, ), lambda *x: [1]),
    'mark61_schedule0_load1':       SelectDefs(lambda *x: (list_testable_vexfiles(), ), lambda *x: [1]),
    'mark61_run0_test-recording2':  SelectDefs(lambda *x: (isint, isint, ), lambda *x: [1, 1]),
    }

#-- constants
constants = {
    'configfile': os.path.join(backendctl_home, CONFIGFILE_FILENAME),
    'actionfile': os.path.join(backendctl_home, ACTIONFILE_FILENAME),
    'backendctl_home': backendctl_home,
    #'becsbin': os.path.join(backendctl_home, BECS_RELATIVE_PATH),
    'becsbin': '/usr/bin/python ' + os.path.join(backendctl_home, BECS_RELATIVE_PATH), #-- can BECS scripts be executable?
    }

import sys
import subprocess
import docparse

rootname = 'backendctl'

#-- parse cmd_doc as Tree
tree = docparse.Tree.from_doc(cmd_doc.split('\n'), rootname)
tree.verify_defs(selectnode_defs, leafnode_defs)
#doc = tree.usage()
#print('Usage:', *doc, sep='\n')

#tree.testsuite(selectnode_defs, leafnode_defs, constants)
#sys.exit()

cmd = sys.argv[1:]

#-- Make level-1 node 'whole' optional
objects = {c.name: None for c in tree.children.values()}
objects.pop('whole')
whole_commands = {c.name for c in tree.children['whole0'].children.values()}
if len(cmd) > 0  and  cmd[0] not in objects  and  cmd[0] in whole_commands:
    print('redirecting to \'whole\' object...')
    cmd = ['whole', *cmd]

err, cmd = tree.resolve(cmd, selectnode_defs, leafnode_defs, constants)

#-- handle resolve error
if err is not None:
    exit(err)

#-- successfully resolved
if DRYRUN:
    print('exec:', cmd)
    exit()

if VERBOSE:
    print('exec:', cmd)
    subp_kwargs = {"stdout": subprocess.DEVNULL, "stderr": sys.stdout}
else:
    subp_kwargs = {"stdout": sys.stdout, "stderr": subprocess.PIPE}
result = subprocess.run(cmd.split(), **subp_kwargs)
if result.returncode == 1 and result.stderr:
    print(result.stderr.decode())
sys.exit(result.returncode)

# vim: foldmethod=indent foldnestmax=1
