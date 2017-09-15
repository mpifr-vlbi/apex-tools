## Copyright 2011 MIT Haystack Observatory
##
## This file is part of Mark6 cplane.
##
## Mark6 cplane is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 3 of the License.
##
## Mark6 cplane is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Mark6 cplane.  If not, see <http://www.gnu.org/licenses/>.

'''
Author:   chester@haystack.mit.edu
Date:     12/7/2012
Description:

This module implements the operational state of the Mark6. It is not
persistent, but only exists while the cplane Server software is running.

State changes are made in respones to commands from the VSI-S command
interface.
'''

import pprint
import thread
import subprocess
import time
import os
import logging
from ctypes import *
from struct import *
import binascii
import Utils
import netinfo
import module_libs
from grp import getgrnam

log = logging.getLogger('M6')

MARK6_EXEC=os.environ.get('MARK6_EXEC', None)
POLLING_INTERVAL=0.1

Version = '1.0.14'  # --- c-plane revision number
CmdSet_rev='1.1' # --- COMMAND SET IMPLEMENTED TO REVISION NUMBER

hw_id_fn="/etc/hardware_id"
hw_exp_id_fn="/etc/hardware_id_exp"
hw_serial_number = 'Mark6-XXXX'
hw_exp_serial_number = 'YYYY'
total_mem = '-'
mnt_meta = "/mnt/disks/.meta/"
mnt_data = "/mnt/disks/"
raid_meta = '/mnt/raid/.meta'
raid_data = '/mnt/raid'
config = "/srv/M6/m6_config.xml"

fn = 'NULL'                      # --- FILE NAME TO STORE XML DATA
ffn = 'NULL'                     # --- FULL PATH AND FILE NAME
ex_last_action = 'finished'      # --- LAST ACTION PROVIDED BY EXECUTE 
m6cc_pid = 0
m6cc_state = 'inactive'


input_streams = dict()
record_session = {'action':'none'}  # --- SET THE DEFAULT RECORD SESSION TO OFF
msn_state = {}
sglist = {}
slot_info = {1:{}, 2:{}, 3:{}, 4:{}}  # --- SLOT INITIALIZATION AND GLOBAL
slot_status = {1:{'group':'-','eMSN':'-','disks':0,'rdisks':0,'rem':'-','cap':0, 'status1':'unknown', 'status2':'null', 'type':'unk', 'msn':'-'},
               2:{'group':'-','eMSN':'-','disks':0,'rdisks':0,'rem':'-','cap':0, 'status1':'unknown', 'status2':'null', 'type':'unk', 'msn':'-'},
               3:{'group':'-','eMSN':'-','disks':0,'rdisks':0,'rem':'-','cap':0, 'status1':'unknown', 'status2':'null', 'type':'unk', 'msn':'-'},
               4:{'group':'-','eMSN':'-','disks':0,'rdisks':0,'rem':'-','cap':0, 'status1':'unknown', 'status2':'null', 'type':'unk', 'msn':'-'}}
slot_raid_mnt = {1:{}, 2:{}, 3:{}, 4:{}}  # --- SLOT MOUNT AND DEVICE LIST FOR RAID0
err_msg_queue = []            # --- ERROR MESSAGE LIST
seqno = 99                    # --- GLOBAL SEQUENCE NUMBER FOR C->D PLANE COMMUNICATION
command_tag = {"abandon_files":0,
               "abort":1, 
               "capture":2, 
               "module_info":3,
               "new_streams":4,
               "outfiles":5,
               "request_module_info":6,
               "request_status":7,
               "status":8,
               "terminate":9,
               "num_commands":10}

unprotect_flag = 0    # --- SET THE GLOBAL UNPROTECT FLAG USED WITH ERASE OF DISK
last_scan_removed = ''

num_devs = 0          # --- NUMBER OF DEVICES MOUNTED IN THE OPEN GROUP FOR RECORDING.
devs_list = []        # --- DEVICE LIST USED WITH D-PLANE OF THE OPEN GROUP, CONTAINS ONLY THE DEV NAME, NOT THE FULL PATH

# --- SET THE GSM_MASK TO all ENABLED, THE DEFAULT
gsm_mask_vals = [1,1,1]

# --- GROUP IDS AVAILBLE
group_mnt = {}       # --- THE MOUNT POINTS AND CORRESPONDING DEVICE MAPPING
open_group = {'slot':'-', 'type':'-'}    # --- THE OPEN GROUP list

# --- INITIALIZE UNASSIGNED SLOTS TO NO SLOTS, ONCE POWER IS APPLIED MOVE FROM NOPOWER TO UNASSIGNED
unassigned_slots = []   
#unassigned_slots = ['1', '2', '3', '4']   # --- INITIALIZE UNASSIGNED SLOTS TO ALL SLOTS 
                                          #     UPDATE DURING INITIZIATION OF C-PLANE FROM XML FILE
# --- INITIALIZE TO NO POWER TO ALL SLOTS, UPDATE WHEN INITIALIZED AND CHECK PERIODICALLY IF 
#     A SLOT HAD POWER ENABLED
nopower_to_slots = ['1', '2', '3', '4']   
fmask = 0                       # --- device mask for sg used with dplane
c_status =0x1                   # --- C-plane status initially system ready

disk_check_prev = 0
disk_poll_int = 6
md5sum_prev = "NULL"
check_disks = 0

file_dict = {}                  # --- DICTIONARY FOR LIST COMMAND
fn_dict = {}                    # --- HASH TABLE FOR FILE NAME TO SCAN ENTRY TO GET COMPLETE DETAILS FROM FILE_DICT
sn = 0                          # --- scan number initialization
restart = 0                     # --- A EXECUTE=RESTART WAS SUCCESULLY ISSUED, RESTART CPLANE, SET TODEFAULT, NO RESTART
exp_name = "expxx"              # --- DEFAULT EXPERIMENT NAME
st_code = "stnxx"               # --- DEFAULT STATION CODE
m6cc_pid = 0
m6cc_state = 'inactive'
subgroup_ref = {0:'0', 1:'0', 2:'0', 3:'0'}

def init():
    global total_mem
    global hw_serial_number
    global nopower_to_slots
    global unassigned_slots

    grp_info = getgrnam("mark6")
    # the group gid is in index 2
    gid = grp_info[2]
    print "setting application gid to ", gid

    # --- SET THE GID TO MARK6 GROUP
    #os.setegid(gid)

    tmp_slot = {0:{}, 1:{},2:{}, 3:{}}
    log.info("State : init() initialization")
    if(os.path.isfile("/srv/M6/m6_config.xml")):
        tree = ET.parse('config_m6.xml')
        root =  tree.getroot()
        for child in root:
            print child.tag, child.attrib

        for stream in root.findall('input_stream'):
            name = stream.get('name') 
            print name
            inter = stream.find('interface').text
            print inter
            pay_size = stream.find('payload_size').text
            pay_off = stream.find('payload_offset').text
            psn_off = stream.find('psn_offset').text
            df = stream.find('data_format').text
            
        log.info("State: Initialized input_stream to last known state")

        for group in root.findall('group'):
            name = group.get('name')
            print name
            for slot in group.iter('slot'):
                # slot_status = {X:{'group':'-','eMSN':'-','disks':0,'rdisks':0,'cap':0, 'status1':'unk', 'status2':'unk'}}
                iname2 = int(slot.get('name'))
                log.info("got information for slot %d of group %s" % (iname2, name))
                tmp_slot[iname2].group = name
                tmp_slot[iname2].status1 = slot.find('status1').text
                tmp_slot[iname2].status2 = slot.find('status2').text
                tmp_slot[iname2].disks = int(slot.find('disks').text)
                tmp_slot[iname2].eMSN = slot.find('eMSN').text
                tmp_slot[iname2].cap = slot.find('storage').text
                print "config info is ", tmp_slot[iname2]

        # --- CALL MODULE_LIST TO GET STATE OF MODULES
        num_disks = module_list()
        if num_disks == 0:
            log.info("State:init - no modules powered up")
        else:
            log.info("State:init - modules powered up, disks available %d" % num_disks)
            for i in range(1,5):
                #print "slot_info[%d] is %s:" % (i, slot_info[i])
                # --- IF SLOT_INFO HAS INFORMATION ON DISKS
                disks_slot = len(slot_info[i])
                # --- IF THERE IS NO INFOMATION AND NO CONFIG INFORMATION WAS STORED, ASSIGN SLOT_STATUS REMAINS INITIAL STATE
                if disks_slot == 0 and tmp_slot[i].disks == 0:
                    log.info("init: no disks in slot %d, slot_state initial values" % (i))
                else:
                    log.debug("init: %d disks in slot %d" % (disks_slot, i))
                    update_slot_list(str(i))

        log.debug("State: verify information of modules to last known data ")

    else:
        log.debug("init: State creating file /srv/M6/m6_config.xml")
        # --- USING UDISK GET ALL INFORMATION ON THE DEVICE
        res = subprocess.Popen(["df","-t", "ext4", "-t", "xfs"],  stdout=subprocess.PIPE)
        out = res.communicate()[0]
        # --- THE OUTPUT IS SEPARATED BY CR"
        mdf = out.split("\n")

        ldf = len(mdf)
        #print "ldf is ", ldf
        tot_size = tot_used = 0

        if ldf > 1:

            for i in range(1, ldf-1):
                (dev, size, used, avail, percent_used, mt_pt) = mdf[i].split()
                tot_size += int(size)
                tot_used += int(used)
                
        log.info("State total disk size mounted %d " % (tot_size))
        log.info("State total mounted disk used %d" % (tot_used))
        log.info("State total mounted available %d" % (tot_size - tot_used))

        # --- CALL MODULE_LIST TO GET STATE OF MODULES
        num_disks = module_list()
        if num_disks == 0:
            log.info("State - no modules powered up")
        else:
            log.info("State - modules powered up, disks available %d" % num_disks)
            for i in range(1,5):
                log.debug("State slot_info[%d] is %s:" % (i, slot_info[i]))
                # --- IF SLOT_INFO HAS INFORMATION ON DISKS
                disks_slot = len(slot_info[i])
                # --- IF THERE IS NO INFOMATION AND NO CONFIG INFORMATION WAS STORED, ASSIGN SLOT_STATUS REMAINS INITIAL STATE
                if disks_slot == 0:
                    log.info("State no disks in slot %d, slot_state initial values" % (i))
                else:
                    log.debug("State %d disks in slot %d, determine slot_status values" % (disks_slot, i))
                    # --- UPDATE AVAILBLE SLOTS FOR ASSIGNEMENT
                    update_slot_list(str(i))

                    # --- REMOVED, SINCE XML IS NOT USED YET
                    #rv = slot_compare(tmp_slot[i], i)

        log.debug("State: verify information of modules to last known data ")

    # --- UPDATE THE MODULE INFORMATION:
    tmp_modules = [1,2,3,4]
    (rv, rl) = module_libs.modules(tmp_modules)
    for i in range(1, 5):
        group_ref = ''
        # --- since get_group expects a list, create if for slot and get its group_info
        tmp = [str(i)]
        # --- CHECK TO SEE IF INFORMATION EXIST ABOUT THE SLOT
        if slot_info[i].keys() and slot_status[i]['type'] == 'sg':
            
            #print "calling get group with %s" % (tmp)
            gg_rv = module_libs.get_group(tmp)
            #print "get_group returned with gg_rv = %s " % gg_rv
            #print "get_group returned with gg_rv[0] = %s " % gg_rv[0]
        
            if len(gg_rv) > 0:
                gg_rv_l = gg_rv[0].split(":")
                #print "the number of modules in this group are %s" % (gg_rv_l[0])

                for j in range(1, 5):
                    #print "slot_status[%d]['eMSN'] = %s"  % (j,slot_status[j]['eMSN'])
                    if slot_status[j]['eMSN'] in gg_rv[0]:
                        #print "the modules eMSN in slot %d is %s and it is a member of %s " % (j,slot_status[j]['eMSN'],gg_rv[0])
                        group_ref += str(j)

                log.debug("State : the group_ref after testing is %s "% group_ref)
                # --- IF AFTER CHECKING, THE GROUP_REF IS THE SAME
                if int(gg_rv_l[0]) == len(group_ref):
                    # --- check if group is mounted
                    if module_libs.slot_mounted(i, 'sg'):
                        #print "update slot_status[%d]s group to %s" % (i, group_ref)
                        slot_status[i]['status1'] = 'closed'
                        slot_status[i]['status2'] = 'unprotected'
                    else:
                        #print "update slot_status[%d]s group to %s" % (i, group_ref)
                        slot_status[i]['status1'] = 'unmounted'
                        slot_status[i]['status2'] = 'unprotected'
                    
                else:
                    hyp = ""
                    for k in range(int(gg_rv_l[0])-len(group_ref)):
                        hyp = hyp+"-"
                    group_ref = group_ref+hyp
                    #print "update slot_status[%d]s group to %s" % (i, group_ref)
                    slot_status[i]['status1'] = 'incomplete'
                    slot_status[i]['status2'] = 'protected'


                slot_status[i]['group'] = group_ref
                log.info("State - setting slots %d to group %s" % (i, group_ref))


                # --- (CHET) Need to check if all data partitions for the group_ref are mounted, if so update the status1 and 2.

    # --- TODO INITIALIZE SERIAL NUMBER OF MARK6 AND EXPANSION CHASIS IF USED.
    if os.path.isfile(hw_id_fn):
        log.debug("State : hardware_id found")
        id_file = open(hw_id_fn, 'r')
        for l in id_file:
            if len(l) > 0:
                cr_loc = l.find("\n")
                if cr_loc > 0:
                    hw_serial_number = l[:cr_loc]
                    log.info("State Mark6 hardware_id is %s" % hw_serial_number)
        log.debug("State closing hardware_id file")
        id_file.close()

    else:
        msg = "File /etc/hardware_id does not exist, please create with the proper ID number"
        add_error_msg(msg)

    if os.path.isfile(hw_exp_id_fn):
        log.debug("State hardware_exp_id found")
        id_file = open(hw_exp_id_fn, 'r')
        for l in id_file:
            if len(l) > 0:
                cr_loc = l.find("\n")
                if cr_loc > 0:
                    hw_exp_serial_number = l[:cr_loc]
                    log.info("State Mark6 hardware_id is %s" % hw_exp_serial_number)
        log.debug("State closing exp_id file")
        id_file.close()
    else:
        log.info("State file /etc/hardware_id_exp does not exist, if you using an expansion chassis, please create with the proper ID number")
        
    total_mem = Utils.TotalMem()
    log.info("State total_mem %s in GB and type is %s " % (total_mem, type(total_mem)))

    return 0
######################################################################
''' SIMPLE ROUTINE TO MOVE SLOTS BETWEEN NO POWER APPLIED AND
    POWERED ON -> UNASSIGNED
'''
def update_slot_list(slot):
    global unassigned_slots, nopower_to_slots

    unassigned_slots.append(slot)
    log.debug("State: appended %s to unassigned slot list" % (slot))
    nopower_to_slots.remove(slot)
    log.debug("State: removed %s from nopower slot list" % (slot))


def add_error_msg(msg):
    global err_msg_queue, c_status

    # --- SET THE ERROR BIT IN THE STATUS WORD
    c_status |= 0x2

    err_msg_queue.append(msg+" | ")
    log.error(msg)


def add_input_stream(stream_label, data_format, payload_offset, payload_size, psn_offset, interface_id, filter_address, port, subgroup):
    global input_streams
    global sglist
    #print "the data format is -%s-" % (data_format)
    # --- DPLANE IS LOOKING FOR EITHER VDIF, MK5B
    if data_format == 'm5b':
        data_format = 'mk5b'

    # --- IF THE SUBGROUP IS DEFINED, MEANING NOT EQUAL TO ZERO
    if subgroup != "0":
        # --- CHECK IF A KEY EXISTS FOR STREAM_LABEL 
        #if stream_label in sglist.keys():
        log.info("subgroup %s was defined for stream %s, adding to sglist " % (subgroup, stream_label))
        

    input_streams[stream_label] = {
                                   'stream_label':stream_label,
                                   'data_format':data_format,
                                   'payload_offset':payload_offset,
                                   'payload_size':payload_size,
                                   'psn_offset':psn_offset,
                                   'interface_id':interface_id,
                                   'filter_address':filter_address,
                                   'port':port,
                                   'subgroup':subgroup
                                   }
    log.debug("State -%s- added " % stream_label)

    return 0

def delete_input_stream(stream_label):
    global input_streams
    log.debug("State - deleting %s"% stream_label)
    if stream_label in input_streams.keys():
        log.debug("State - %s if a valid key", stream_label)
        del input_streams[stream_label]
        return 0
    else:
        msg = "State s% is not in input_stream" % stream_label
        # --- log into message queue
        add_error_msg(msg)
        return 1

def get_input_stream(stream_label=None):
    global input_stream
    log.debug("State ")    
    if stream_label:
        return [ input_streams.get(stream_label, None) ]
    return input_streams.values()

def commit_input_stream():
    global input_streams
    global c_status
    global subgroup_ref

    log.debug("State - commiting the following streams")    
    # --- used for debugging.
    #dump()

    # --- GET THE KEYS
    s = get_input_stream()
    log.debug("State - got input_stream values")    

    # --- get the number of streams
    num_streams = len(s)
    log.debug("State - number of streams to commit %d" % num_streams)
    rec = create_string_buffer(332)
    log.debug("State - created %d byte message" % (len(rec)))
    pack_into('IIi',rec, 0,
              command_tag['new_streams'],
              seqno,
              num_streams)
    log.debug("State - added %d, seqno %d" % (command_tag['new_streams'], seqno))

    i=0
    for e in s:
        ''' 
        To determine the buffer offset used:
                  struct.calcsize('IIi')  + ( struct.calcsize('16s8sii16siiiiiii') * i)
        which represents the packet we are sending.
        '''
        buff_offset = 12+(80*i)

        log.debug("State - adding next stream at offset %d" % buff_offset)
        log.debug("State - for interface  %s, format %s, size %s, offset %s psn_offset %s, subgroup %s" % (e['interface_id'],
                                                                                                           e['data_format'],
                                                                                                           e['payload_size'], 
                                                                                                           e['payload_offset'],
                                                                                                           e['psn_offset'],
                                                                                                           e['subgroup']))
        
        
        subgroup_ref[i] =  e['subgroup']
        log.info("Setting input stream %d subgroup_ref to %s" % (i, subgroup_ref[i]))

        pack_into('16s8siii16siiiiiii', 
                  rec, 
                  buff_offset,
                  e['interface_id'],
                  e['data_format'],
                  int(e['payload_offset']),                # --- e['packet_len'], RJC is using 42 bytes for UDP header, etc.
                  int(e['payload_size']),
                  int(e['psn_offset']),
                  e['interface_id'],
                  0,0,0,0,0,0,        # --- SOURCE IP IS PRESENTLY IGNORED
                  4206                # --- e['port']
                  )
        i += 1

    for j in range(i,4):
        buff_offset = 12+(72*j)

        log.debug("State - adding next stream at offset %d" % buff_offset)
        log.debug("State - for interface  %s, format %s" % ('0', ''))
        pack_into('16s8si16siiiiiii', rec, buff_offset,
                  '0',
                  '',
                  0,                # --- e['packet_len'], RJC is using 42 bytes for UDP header, etc.
                  '',
                  0,0,0,0,0,0,        # --- SOURCE IP IS PRESENTLY IGNORED
                  0                # --- e['port']
                  )


    log.debug("State - sending info to d-plane")
    rv = Utils.d_plane_comm(0 ,"input_stream", rec)
    log.debug("State - d-plane-comm returned %d " % rv)

    # --- IF THE DPLANE OPERATIONAL BIT IS NOT SET
    if not ( c_status & 0x100 ):
        # --- CHECK IF DPLANE IS UP
        rv = Utils.dplane_status()
        if not rv:
            msg = "d-plane-comm returning error, since d-plane is not running and command not processed"
            add_error_msg(msg)            
            return (4)
        else:
            log.warning("State, communication for commiting input stream was made, please verify results")

    if not ( c_status & 0x300 ):
        c_status |= 0x200
        log.info("State , dplane updated to handle incoming data")

    return (0)

def is_commit():
    log.debug("State ")
    # config.update_is(input_streams)


def dump():
    global input_streams
    
    p = pprint.PrettyPrinter()
    p.pprint(input_streams)

def record(ts, delta, name, *args):
    global seqno
    global command_tag

    #print ts
    #print delta
    #print name
    log.debug("State start_time is %s ", ts)
    log.debug("State duration is %s ", delta)
    #end_time = record_session['start_time'] + record_session['duration']

    st = time.mktime(ts)
    log.debug("State start time is %f", st)
    end_time = st + int(delta)
    log.debug("State end time is now %f", end_time)

    pt = time.mktime(time.gmtime())
    log.debug("State preset time is now %f", pt)

    if pt > end_time:
        log.warning("State time is > end_time - exception ")
        raise Exception('late scan: %s'% name )
    else:
        log.debug("State time is < end_time ")

    log.debug("State Calling time.mktime(time.gmtime())")
    pt = time.mktime(time.gmtime())
    #print "time is %f", pt

    log.debug("State d-plane-comm ")
    Utils.d_plane_comm(0 ,"record", rec)
    log.debug("State leaving ")

################################# record=on #####################################
def on_record_session(action, vtime, start_time, duration, data_size, scan_name,
    experiment_name, station_code):
    global record_session
    global seqno
    global command_tag
    global input_streams  # --- require input_streams[stream_label]['data_format']
    global subgroup_ref

    log.debug("State Enter with action %s" % (action))
    
    record_session['action'] = action
    record_session['vex_time'] = vtime
    record_session['start_time'] = start_time
    record_session['duration'] = duration
    record_session['data_size'] = data_size
    record_session['scan_name'] = scan_name
    record_session['experiment_name'] = experiment_name
    record_session['station_code'] = station_code

    #pt = time.mktime(time.gmtime())
    pt = time.time()
    log.debug("State present time is now %f", pt)

    # --- SEND THE OUTPUT FILE INFORMATION
    # --- CREATE THE PACKET
    log.debug("State create msg buffer len 284")    
    omsg = create_string_buffer(284)

    # --- GET THE KEYS
    s = get_input_stream()
    log.debug("State - got input_stream values")    

    # --- DETERMINE the number of streams
    num_streams = len(s)

    # --- GENERATE FILENAME as EXPCODE__STATIONCODE_SCANNAME.filetype
    log.debug("State exp %s, station %s, scanname %s filetype %s" % (experiment_name,station_code,scan_name,"vdif"))

    if num_streams < 1:
        msg = "State error the number of streams - invalid %d" % num_streams
        add_error_msg(msg)
        return(0)
    else:
        log.info("State - the number of streams  %d" % (num_streams))

    # --- GET THE PACKET TYPE
    i = 0
    for e in s:
        if i == 0:
            file_ext = e['data_format']
            log.info("State - file extension being set to %s " % (file_ext))
            i += 1    
        else:
            if file_ext != e['data_format']:
                msg = "State error the data_formats are not the same - invalid condition %s vs %s" % (file_ext, e['data_format'])
                add_error_msg(msg)
                return(0)
            else:
                log.debug("State data_formats are identical %s " % (file_ext))


    # --- INITIALIZE THE NUMBER OF FILES 
    nfiles = 0
    # --- INITIALIZE THE BASENAME AND MOUNT POINTS 
    #mount_dev = []
    #for i in range(32):
    #    mount_dev.append('')

    final_scan_name_no_ext = experiment_name + "_" + station_code + "_" + scan_name
    final_scan_name = 'data/'+final_scan_name_no_ext + ".vdif"
    log.debug("State final_scan_name %s" % (final_scan_name))

    # --- SUPPRESS HEADERS OF SCATTER GATHER, LATER THIS WILL BE AN OPTION
    if open_group['type'] == 'sg':
        mode = 1
    else:
        mode = 2

    log.debug("State : create msg %d, seqno %d" % (command_tag['outfiles'], seqno))

    # --- SET UP THE HEADER INFO 
    pack_into('IIi', omsg, 0, command_tag['outfiles'], seqno, mode)
    loc = 12
    #print "loc ", loc
    b_fmask = bin(fmask)
    #print "fmask is now ", fmask
    # --- REMOVE THE OB AT THE BEGINNING
    b_fmask = b_fmask[b_fmask.find('b')+1:]
    #print "b_fmask ", b_fmask
    if b_fmask.count('1') != len(devs_list):
        # --- THE MASK DOES NOT MACH NUMBER NUMBER OF ITEMS IN THE DEV LIST.  A DEVICE
        #     MAY HAVE FAILED, RECALCULATE FMASK
        
        log.info(" State : NEED TO re-adjusted fmask" )

    fmask_dict = {0:0, 1:0, 2:0, 3:0}
    # --- IF SUBGROUP_REF IS ZERO, SET FMASK[0] AND ZERO OUT THE REST
    log.info("subgroup_ref %s" % subgroup_ref)
    i = 0
    for slabel in input_streams.keys():    
        log.info("adding fmask for %s subgroup_ref[%d]=%s" % (slabel, i, subgroup_ref[i]))
        if subgroup_ref[i] == '0':
            fmask_dict[i] = fmask
        # --- ELSE DETERMINE THE FMASK BASED UPON SUBGROUP_REF
        else:
            # --- ASSIGN GLOBAL VALUES SET (NEED TO DETERMINE HEX VALUE
            lparam = list(subgroup_ref[i])
            # --- SORT THE LIST IN ORDER
            lparam.sort()
            log.debug("State: subgroup %s " % lparam)
            # --- NUMBER OF SLOTS BEING GROUPED  _CHET_ NEED TO THINK ABOUT HOW TO BUILD MASK
            for j in lparam:
                if j == '1':
                    fmask_dict[i] |= (fmask & 0xff)
                if j == '2':
                    fmask_dict[i] |= (fmask & 0xff00)
                if j == '3':
                    fmask_dict[i] |= (fmask & 0xff0000)
                if j == '4':
                    fmask_dict[i] |= (fmask & 0xff000000)
        i+=1

    log.debug("fd %s" % fmask_dict)
    # --- WRITE OUT THE FMASK
    pack_into('I', omsg, loc, fmask_dict[0])
    loc += 4
    pack_into('I', omsg, loc, fmask_dict[1])
    loc += 4
    pack_into('I', omsg, loc, fmask_dict[2])
    loc += 4
    pack_into('I', omsg, loc, fmask_dict[3])
    loc += 4

    
    #print "loc2 ", loc

    # --- WRITE OUT THE FINAL SCAN NAME
    pack_into('256s', omsg, loc, final_scan_name)
    #loc += 256

    #print "loc3 ", loc
    # --- REMOVED IN MESSAGES.H SO COMMENTED OUT.
    #if open_group['type'] == 'sg':
    #    for slot in open_group['slot']:
    #        islot = int(slot) - 1 
    #        #print "islot ", islot
    #        # --- CALC THE STARTING LOCATION FOR THE DEVNAMES
    #        base_loc = loc + (islot * 64)
    #        #print "base_loc ", base_loc
    #        for i in range(slot_status[int(slot)]['rdisks']):
    #            # --- SET THE MOUNT DEVICES OF DIRECTORIES 
    #            log.debug("State : add mount points %s, at 0x%x " % (devs_list[i], base_loc))
    #            pack_into('8s', omsg, base_loc, devs_list[i])
    #            # --- since we are at the proper located when we enter the for loop, increment to the next location
    #            base_loc  += 8
    #
    #else:
    #    # --- NEED TO CORRECT MOUNT_DEV.  iT IS NOT BEING USED ANY MORE CHET
    #    log.debug("State : add mount points %s, " % (mount_dev[0]))
    #    loc+=(int(slot)-1)*64
    #    pack_into('8s', omsg, loc, mount_dev[0])


    #print 'State.on_record_session: message being sent for outfiles', binascii.hexlify(omsg)
    log.debug("State : sending outfile to d-plane-comm ")
    rv = Utils.d_plane_comm(0 ,"outfile", omsg)
    log.debug("State returned from d_plane_comm %d " % rv)

    # --- WAIT FOR 1 SEC BEFORE SENDING CAPTURE COMMAND
    time.sleep(1.0)

    if rv == 0:
        if action == 'on' or action == 'off':
            # --- SET THE START CONTROL TO DPLANE TO IMMEDIATE
            start_control = 0
            # --- SET THE VDIF EPOCH TIME TO 0
            epoch_time = 0
            # --- WILL WANT TO GET WALL TIME CONVERT TO VDIF TIME
            #st = time.mktime(start_time)
            st = 0
            log.debug("State start time is %f", st)
            end_time = st + int(duration)
            log.debug("State end time is now %f", end_time)
            # --- BECAUSE THE PRESENT TIME IS ALWAYS > 0 + DURATION, REMOVE THIS WARNING
            #if pt > end_time:
            #    log.warning("State time is > end_time - exception ")
            ##raise Exception('late scan: %s'% name )
            #else:
            #    log.info("State time is < end_time ")
        else:
            if start_time == 0:
                log.debug("State start time is 0")
                st = 0
    # --- FOR IMMEDIATE
            else:
                st = start_time
                start_control = 3 # --- for vdif

                # --- SINCE WE HAVE A VEX TIME THAT WAS PASSED GET THE EQUIVLENT VDIF TIME
                (epoch, epoch_time) = Utils.vex2vdif(vtime)
                log.info("State epoch_time in seconds is %f" % (epoch_time))

        # --- set the msg_type to 1 for record
        msg_type = 1 
        log.debug("State create msg buffer len 104")    
        rec = create_string_buffer(104)
        log.debug("State create msg %d, seqno %d" % (command_tag['capture'], seqno))

        pack_into('IIii23sii32s8s8sii', rec, 0,
                  command_tag['capture'],
                  seqno,
                  start_control,    
                  pt,
                  '0',              # --- VEX TIME, CONVERTED TO VDIF TIME ALL CASES, HENCE 0
                  int(epoch_time),  # --- VDIF REF EPOCH
                  duration,
                  scan_name,
                  experiment_name,
                  station_code,
                  1,                # --- WRITE DATA TO FILE
                  1)                # --- CLOSE FILE WHEN CAPTURE TERMINATES

        #print 'message being sent is', binascii.hexlify(rec)
        #time.sleep(2)
        log.debug("State d-plane-comm ")
        rv = Utils.d_plane_comm(0 ,"record", rec)
        log.debug("State d_plane_comm returned %d " % rv)

        if action != 'off':
            # --- ADD LISTING ONTO FILE_DICT  add_file_to_listing(sl, status, start, duration, num_streams, spc)
            lrv = module_libs.add_file_to_listing(final_scan_name_no_ext, 'pending', start_time, duration, num_streams, 0)

        #if action == 'off':
        #    lrv = module_libs.commit_file_listing(gref, sn, status)

    return rv
    #record(start_time,duration,scan_name)

def off_record_session():
    global record_session
    global seqno

    msg_type = 1 
    action = "abort"
    
    record_session['action']= 'off'
    
    rec = create_string_buffer(12)
    pack_into('IIi', rec, 0,
              command_tag["abort"],
              seqno,
              1)      # --- DRAIN BUFFERS AFTER CAPTURE STOPS
    
    log.debug("State d-plane-comm ")
    rv = Utils.d_plane_comm(0 ,"record", rec)
    log.debug("State d_plane_comm returned %d " % rv)
    log.debug("State leaving ")
    
    return rv 
    #record_session = dict()
    
def get_record_session():
    global record_session
    return record_session

def m6_net_interfaces():

    log.debug("State Enter")    

    net_int = {}

    # --- GET LIST OF ACTIVE INTERFACES
    active_devs = netinfo.list_active_devs()

    # --- GET LIST OF All INTERFACES
    devs = netinfo.list_devs()

    # --- IF THE NUMBER OF INTERFACES ARE THE SAME, ALL ARE CONFIGURED
    if (len(active_devs)) != (len(devs)):
        log.debug("State All interfaces are not configure")
    #print devs

    for dev in devs:
        log.debug("State %s being processed" % dev)
        if not(dev == "lo"):
            if dev in active_devs:
                log.debug("State %s is in active list" % dev)
                state ="up"
                ip = netinfo.get_ip(dev)
                log.debug("State ip is %s" % ip)                
                log.debug("State state is %s" % state)
            else:
                state = "down"
                ip = "-"
                log.debug("State ip is %s" % ip)                
                log.debug("State state is %s" % state)

            log.debug("State state is %s" % state)
            log.debug("State ip is %s" % ip)                
            # --- NEED TO GET THE SPEED OF THE INTERFACES
            log.debug("State calling netinfo.speed with -%s-" % dev)
            (rv, speed) = netinfo.speed(dev)
            log.debug("State returned with rv %d speed is %d" % (rv, speed))

            # --- speed = 
            net_int[dev] = {'state':state, 'ip':ip, 'speed':str(speed)}
            #print net_int.keys()

        else:
            log.debug("State %s is skipped" % dev)

    log.debug("State Exit")    
    return(net_int)
        
# --- module_state() returns the Modules that are available
def module_list():
    global msn_state
    global slot_info

    # --- number of module enteries 
    lmsn = len(msn_state)
    lslot = len(slot_info)
    host_keys = []

    
    t_msn_state = Utils.disks()
    lt_msn_state = len(t_msn_state)
    log.debug("State length of t_msn_state is %d" % lt_msn_state)
    
    # --- if there are no entries, determine if any are available
    if lmsn == 0 or lslot == 0:
        log.debug("State has no modules available")
        log.debug("State looking for powered up modules")

        # --- SINCE LMSN=0, ASSIGN T_MSN_STATE KEYS
        msn_state=t_msn_state

        host_keys = msn_state.keys()
        log.debug("State msn_state keys are %s " % (host_keys))

        host_keys.sort()
        log.debug("State msn_state sorted keys based on host id  are %s " % (host_keys))

        # --- ASSIGN THE DISK TO THE SPECIFIC SLOT THEY ARE IN:  HERE SLOT REFERS TO DEV NUMBER
        for slot, disks in msn_state.iteritems():
            if slot == host_keys[0]:
                for i in disks:
                    log.debug("State msn_state[%d][%i] = %s" %(slot,i, msn_state[slot][i]))
                    #print msn_state[slot][i]
                    if i < 8:
                        slot_info[1][i] = msn_state[slot][i]
                    else:
                        slot_info[2][i] = msn_state[slot][i]                        
            else:
                for i in disks:
                    log.debug("State msn_state[%d][%i] = %s" %(slot,i, msn_state[slot][i]))
                    if i < 8:
                        slot_info[3][i] = msn_state[slot][i]
                    else:
                        slot_info[4][i] = msn_state[slot][i]                        

        rv = 0
        for i in range(1,5):
            log.debug("State slot_info[%d] is %s:" % (i, slot_info[i]))
            #print ("State.module_list slot_info[%d] is %s:" % (i, slot_info[i]))
            #print ("len of slot_info[%d] is %d" % (i, len(slot_info[i])))
            rv +=len(slot_info[i]) 
            #print "rv is now %d" % (rv)

    #--- ELSE LOOK TO SEE IF ANY MODULES ARE POWERED UP
    else:
        log.debug("State has modules available")
        rv = 0
        for i in range(1,5):
            log.debug("State slot_info[%d] is %s:" % (i, slot_info[i]))
            #print ("len of slot_info[%d] is %d" % (i, len(slot_info[i])))
            rv += len(slot_info[i]) 

    log.debug("State returning from module_list with rv = %d" %(rv))
    return rv 




# --- msn_info() gets the info_type for all MSN, or a specific MSN
def module_info(itype, slot_num):
    global msn_state   # --- CONTAINS ALL OF THE DISKS IN THE SYSTEM
    global slot_info   # --- CONTAINS THE DISKS LOCATED IN THE SPECIFIC SLOT
    msndict = {}

    rv = 1

    log.debug("State Entered with slot_num %d, type = %s "%(slot_num, itype))
    #msndict = Utils.disks()
    module_list()
    log.debug("State module_list() returned")
    # --- IF SLOT IS -1, THEN GET ALL
    if slot_num == -1:
        num_disks_in_slot = len(slot_info[1]) + len(slot_info[2]) + len(slot_info[3]) + len(slot_info[4])        
    else:
        num_disks_in_slot = len(slot_info[slot_num])
        #print "slot_info is  ",slot_info[slot_num]

    log.debug("State number of disks in slot %d is %d", slot_num, num_disks_in_slot)

    # --- CHET THINK ABOUT HOW TO STORE AND RETURN THIS INFORMATION.  SHOULD IT BE ON EACH DISK DRIVE
    # --- INITIALIZED DURING MOD_INIT?
    if num_disks_in_slot == 0:
        msndict[slot_num] = ["-","-","-","-","-","-","-","-"]
        return msndict
    else:
        log.debug("State information in slot_num %d exists" % (slot_num))

    if itype in ["usage", "serial", "model", "size", "temp", "vendor"]:
        log.debug("State:msn_info() getting %s" % (itype))
        msndict = disk_info(itype, slot_num)
        #print "state msndict is :", msndict
    else :
        msg = "State:msn_info() illegal type"
        add_error_msg(msg)
        rv = 0

    return msndict

# --- TERMINATE D-PLANE
def dterm():
    global seqno
    global command_tag
    log.debug("State entered ")    

    rec = create_string_buffer(8)
    log.debug("State created 8 byte message")
    pack_into('II',rec, 0,
              command_tag['terminate'],
              seqno
              )
    log.debug("State - added %d, seqno %d" % (command_tag['terminate'], seqno))
    log.debug("State - send info to d-plane")
    rv = Utils.d_plane_comm(0 ,"terminate", rec)
    log.debug("State - d-plane-comm returned %d " % rv)
    return(rv)

######################################################
# --- GROUP FUNCTION FOR MANAGING MODULES 
def group(act, param):
    global slot_status
    global open_group
    global file_dict, fn_dict
    global sn
    global unprotect_flag

    # --- INITIALIZE GROUP_REF 
    group_ref=''
    grp_req = ''.join(param)
    group_type = '-'
    update_state = 1
    log.debug("State - Entered with %s, %s for %s" % (act, param, grp_req))    
    
    if act == 'open':
        log.debug("State : (%s) a group_ref %s for recording" % (act, param))    
        for i in param:
            slot = int(i)
            log.debug("State : slot_status type is %s" % slot_status[slot]['type'])
            if slot_status[slot]['type'] == 'sg':
                group_type = 'sg'
                # --- CHECK TO SEE IF IT IS MOUNTED, IT IS SUPPOSE TO BE, IF NOT, MOUNT IT.
                if slot_status[slot]['status1'] != 'mounted':
                    update_status = 0
                    log.debug("State : Mount the modules in slot %s as SG" % i)
                    dev_start = Utils.si_start(slot)
                    log.debug("State : Mount all the disks in slot %s from %d" % (i, dev_start))                
                    trv = module_libs.mount_module('1', i, 'all',dev_start)
                    log.debug("module_libs : mount_module returned %d"% trv)
                    trv = module_libs.mount_module('2', i, 'all',dev_start)
                    log.debug("module_libs : mount_module returned %d"% trv)

                    if trv:
                        update_status = 1

                if update_status:
                    slot_status[slot]['status1'] = 'open'
                    slot_status[slot]['status2'] = 'ready'
                    log.debug("State :  Updating status1 to %s " % (slot_status[slot]['status1']))
                    log.debug("State :  Updating status2 to %s " % (slot_status[slot]['status2']))
                    group_ref = group_ref+i

            # --- ELSE IT IS A RAID 
            elif slot_status[slot]['type'] == 'raid':
                group_type = 'raid'
                if slot_status[slot]['status1'] != 'mounted':
                    update_status = 0
                    log.debug("State : Mounting the modules in slots %s as a RAID 0" % i)            
                    trv = module_libs.raid_mnt('mount', slot)
                    log.debug("State :  raid_mnt returned %d" % trv)
                    if trv == 1:
                        update_status = 1

                if update_status:
                    group_ref = str(slot)
                    open_group['type'] = 'raid'
                    open_group['slot'] = group_ref
                    slot_status[slot]['group'] = group_ref
                    log.debug("State :  assigning slot %d group_ref %s " % (slot, slot_status[slot]['group'] ))
                    log.debug("State : slot_status is %s" % slot_status[slot])
                    slot_status[slot]['status1'] = 'open'
                    slot_status[slot]['status2'] = 'ready'
                    log.debug("State :  Updating status1 to %s " % (slot_status[slot]['status1']))
                    log.debug("State :  Updating status2 to %s " % (slot_status[slot]['status2']))

            else:
                msg = "Disks in slot %s are not configured as a RAID or SG but %s, group failed" % (i, slot_status[slot]['type'])
                add_error_msg(msg)
                rv = {'rv':'2', 'gref':'-'}
                return(rv)

        # --- REQUIRED SINCE i ASSUME A RAID IS A GROUPT OF ONE MODULE
        if group_type == 'sg':
            open_group['type'] = group_type
            group_ref = str(group_ref)
            open_group['slot'] = group_ref
            log.debug("State : open_group[type] is %s, open_group[slot] is %s " % (open_group['type'],open_group['slot']))

        # --- GET THE DIRECTORY LISTING FROM DISK MODULE
        file_dict = module_libs.read_list(group_ref)
        log.debug("read list of scans for %s " % group_ref)

        # --- CHECK THE RETURN TO SEE THE LENGTH OF FILE_DICT
        if len(file_dict.keys()) > 0:
            scan_nums = sorted(file_dict.keys())
            last_scan_loc = len(scan_nums) - 1
            sn = scan_nums[last_scan_loc]
            log.info("Scan listing was successfully read, last scan number %d", sn)

            # ---UPDATE FN_DICT FOR CROSS REFERENCE OF NAME TO SCAN_NUM
            for i in scan_nums:
                fn_dict[file_dict[i]['sn']] = i
            
            #print "fn_dict ", fn_dict 

            # --- SEE IF SLIST IS STORED IN THE META DATA.
            lg = list(group_ref)
            
            fn = mnt_meta + lg[0] + "/"+str(1) +"/slist"
            log.debug("does %s exist " % fn)
            # --- IF THE FILE SLIST DOES NOT EXIST ON THE META PARITIONS WRITE IT
            if not os.path.isfile(fn): 
                module_libs.write_list(group_ref, "new", 0)
        else:
            sn = 0
            log.info("Scan listing does not exist")
          
        log.info("State : group opened for %s" % group_ref)
        rv = {'rv':'0', 'gref':group_ref}

    elif act in ['auto', 'new', 'mount'] :
        log.debug("State (%s) create a group from %s available modules " % (act, param))
        # --- THE SLOTS ARE UNASSIGNED AND POWERED UP, VERIFIED IN COMMAND:GROUP

        # --- IF THE LENGTH IS GREATER THAN ONE OF LIST, ASSUME SG
        if act in ['auto', 'new'] :

            if len(param) > 1 :
                # --- IF IT IS A SG DEVICE THEN CREATE THE GROUP CONTENTS AND WRITE THE META DATA
                log.debug("State : Creating group meta data and writing it for %s" %param)
                ml_rv = module_libs.create_group(param)
                log.debug("State : create_group returned %d " % ml_rv)

            # --- ELSE IF LENGTH IS 1
            elif len(param) == 1:
                log.debug("State : len(param) == 1 and param is %s" %  param)
                g_islot = int(param[0])
                log.debug("State : slot_status - type is %s" % slot_status[g_islot]['type'] )
                # --- CHECK THE TYPE OF MODULE IS SG
                #print "slot_status[int(param)]['type'] is ", slot_status[int(param[0])]['type']
                if slot_status[int(param[0])]['type'] == 'sg': 
                    # --- IF IT IS A SG DEVICE THEN CREATE THE GROUP CONTENTS AND WRITE THE META DATA
                    log.debug("State : Creating group meta data and writing it for %s" %param)
                    ml_rv = module_libs.create_group(param)
                    log.debug("State : create_group returned %d " % ml_rv)
                # --- ELSE WE HAVE TO MOUNT THE MODULE, THEN WRITE THE META DATA
                else:
                    log.debug("Mounting the modules in slots %d as a RAID 0" % g_islot)            
                    trv = module_libs.raid_mnt('mount', g_islot)
                    if trv == 1: 
                        group_ref = param[0]
                        slot_status[g_islot]['status1'] = 'closed'
                        slot_status[g_islot]['status2'] = 'unprotected'
                        log.debug("State :  Updating status1 to %s " % (slot_status[g_islot]['status1']))
                        log.debug("State :  Updating status2 to %s " % (slot_status[g_islot]['status2']))
                        
                        log.debug("State : Creating group meta data and writing it for %s" %param)
                        ml_rv = module_libs.create_group_raid(param)
                        log.debug("State : create_group_raid returned %d " % ml_rv)
                        ml_rv = module_libs.update_raid(param)
                        log.debug("State : update_raid returned %d " % ml_rv)
                    
        for i in param:
            slot = int(i)
            log.debug("State : slot_status type is %s" % slot_status[slot]['type'])
            if slot_status[slot]['type'] == 'sg':

                log.debug("Mount the modules in slot %s as SG for group_ref %s" % (i, grp_req))
                if group_ref == '-': group_ref = i 
                else: group_ref += i
                # --- MOUNT ALL
                dev_start = Utils.si_start(slot)

                log.debug("State : mount_module in slot %s, all, %d " % (i,dev_start))
                ml_rv = module_libs.mount_module('1', i, 'all', dev_start)
                log.debug("State : mount_module returned from slot %s, all, %d with %d" % (i,dev_start, ml_rv))
                trv = module_libs.mount_module('2', i, 'all',dev_start)
                # --- for new devices, make sure the eMSN is written along with the group and serial info
                log.debug("module_libs : mount_module returned %d"% trv)
                #module_libs.set_eMSN()
                if trv:
                    
                    log.debug("group reference is now %s for SG" % group_ref)
                    slot_status[slot]['status1'] = 'closed'
                    slot_status[slot]['status2'] = 'unprotected'
                    log.debug("State :  Updating status1 to %s " % (slot_status[slot]['status1']))
                    log.debug("State :  Updating status2 to %s " % (slot_status[slot]['status2']))

            # --- ELSE IT IS A RAID 
            elif slot_status[slot]['type'] == 'raid':
                if act != 'new':
                    log.debug("Mounting the modules in slots %d as a RAID 0" % slot)            
                    trv = module_libs.raid_mnt('mount', slot)
                    if trv == 1: 
                        group_ref = str(slot)
                        slot_status[slot]['status1'] = 'closed'
                        slot_status[slot]['status2'] = 'unprotected'
                        log.debug("State :  Updating status1 to %s " % (slot_status[slot]['status1']))
                        log.debug("State :  Updating status2 to %s " % (slot_status[slot]['status2']))

            else:
                msg = "Disks in slot %s are not configured as a RAID or SG but %s, group failed" % (i, slot_status[slot]['type'])
                add_error_msg(msg)
                rv = {'rv':'2', 'gref':'-'}
                return(rv)

        log.info("State - group created for %s" % group_ref)
        rv = {'rv':'0', 'gref':group_ref}

    elif act == 'close':
        log.debug("State (close) group_ref %s " % (param))
        group_ref = ''
        for i in param:
            slot = int(i)
            log.debug("State : slot_status type is %s" % slot_status[slot]['type'])
            if slot_status[slot]['type'] == 'sg':
                log.debug("State : State changed to close for the modules in slots %s as SG" % i)
                # --- WE WILL NOT UNMOUNT THE SG MODULES, SIMPLY CHANGE THE STATE, UNMOUNT PERFORMS UNMOUNT
                slot_status[slot]['status1'] = 'closed'
                slot_status[slot]['status2'] = 'unprotected'
                group_ref += str(slot)
                
                
                
        # --- ELSE IT IS A RAID 
            elif slot_status[slot]['type'] == 'raid':
                log.debug("State : Closing the modules in slots %s as a RAID 0" % i)            
                trv = module_libs.raid_mnt('close', slot)
                log.debug("State : raid_mnt returned %d for close " %trv)
                if trv == 1: 
                    slot_status[slot]['status1'] = 'closed'
                    slot_status[slot]['status2'] = 'unprotected'
                    group_ref = str(slot)
                    log.debug("State : setting gref to %s " % group_ref)
                    
                else:
                    msg = "Disks in slot %s are not configured as a RAID or SG but %s, group failed" % (i, slot_status[slot]['type'])
                    add_error_msg(msg)
                    rv = {'rv':'2', 'gref':'-'}
                    return(rv)

                #group_ref += i

        
        log.info("State - group closed  for %s" % group_ref)
        rv = {'rv':'0', 'gref':group_ref}        
        
    # --- FEATURE TO ADD, FOR PROTECT, MAY WANT TO CHANGE THE THE META DATA TO REFLECT STATE.
    elif act == 'protect':
        log.debug("State (potect) group_ref %s" % (param))    
        for i in param:
            slot = int(i)
            # --- PROTECT AUTOMATICALLY CLOSES AN OPEN GROUP AND PROTECTS IT.
            slot_status[slot]['status1'] = 'closed'
            slot_status[slot]['status2'] = 'protected'
            open_group['slot'] = '-'
            open_group['type'] = '-'
            group_ref += str(slot)
        rv = {'rv':'0', 'gref':group_ref}

    elif act == 'unprotect':
        log.debug("State (unprotect) enable group_ref %s " % (param))  
        unprotect_flag = 1
        for i in param:
            slot = int(i)
            slot_status[slot]['status2'] = 'unprotected'
            group_ref += str(slot)
        rv = {'rv':'0', 'gref':group_ref}

    elif act == 'erase':
        log.debug("State (erase) all of group_ref %s " % (param))    
        rv = {'rv':'2', 'gref':param}

    elif act == 'unmount':
        group_ref = ''
        log.debug("State (unmount) group_ref %s " % (param))    
        for i in param:
            slot = int(i)
            log.debug("State : slot_status type is %s" % slot_status[slot]['type'])
            if slot_status[slot]['type'] == 'sg':
                log.debug("unmount the modules in slot %s as SG" % i)
                ret = module_libs.sg_unmount(i)
                log.debug("State : sg_unmount returned  %d " % ret)                
                if ret :
                    slot_status[slot]['status1'] = 'unmounted'
                    slot_status[slot]['status2'] = 'null'
                    group_ref += i
                
            # --- ELSE IT IS A RAID 
            elif slot_status[slot]['type'] == 'raid':
                log.debug("Closing the modules in slots %s as a RAID 0" % i)            
                trv = module_libs.raid_mnt('unmount', slot)
                if trv == 1: 
                    group_ref = str(slot)
                    slot_status[slot]['status1'] = 'unmounted'
                    slot_status[slot]['status2'] = 'null'
                else:
                    msg = "Disks in slot %s are not configured as a RAID or SG but %s, group failed" % (i, slot_status[slot]['type'])
                    add_error_msg(msg)
                    rv = {'rv':'2', 'gref':'-'}
                    return(rv)

        log.info("State - group closed  for %s" % group_ref)
        rv = {'rv':'0', 'gref':group_ref}        
    # --- 

    return(rv)

##############################################3
def set_gsm_mask(eme, pme, rme):

    log.info("State setting the gsm_mask")

def get_gsm_mask():

    log.info("State getting the gsm_mask")

    rv = 0
##############################################3
# RETURN SPECIFIC USAGE INFO FOR TYPE IN SLOT   
def disk_info(infotype, num):
    global slot_info
    rlist = {}
    
    log.debug("State : (%s, %d)" % (infotype, num))

    if infotype == "serial":
        infotype = "serial_num"

    # --- IF SLOT IS 1,3 DISKS 0-7
    if num == -1:
        for i in range (1,5):
            if len(slot_info[i]) != 0:
                log.info("State : disks in slot %d" % ( i))
                rlist[i] = []
                log.debug("State : (%s, %d) rlist[%d] " % (infotype, num, i))
                if i % 2:
                    for j in range (0,8):
                        #log.debug("State : %s" % (slot_info[i][j][infotype]))
                        rlist[i].append(slot_info[i][j][infotype])
                        #log.debug("State : disk_info(%s, %d) rlist[%d] is %s" % (infotype, num, i, rlist[i]))            
                        #print rlist[i]
                else:
                    for j in range (8,16):
                        #log.debug("State : %s" % (slot_info[i][j][infotype]))
                        rlist[i].append(slot_info[i][j][infotype])
                        #log.debug("State : disk_info(%s, %d) rlist[%d] is %s" % (infotype, num, i, rlist[i]))            
                        #print rlist[i]
            else:
                log.info("State : no disks in slot %d" % (i))

        return rlist

    else:
        if len(slot_info[num]) != 0:
            rlist[num] = []
            log.info("State : disks in slot %d", num)
            if num % 2:
                log.debug("State : disk_info(%s, %d) bf: rlist[%d] is %s " % (infotype, num, num, rlist[num]))            
                for j in range (0,8):
                    #log.debug("State : num %d cnt %d %s" % (num,j, slot_info[num][j][infotype]))
                    rlist[num].append(slot_info[num][j][infotype])
                    #log.debug("State : disk_info(%s, %d) rlist[%d] is %s" % (infotype, num, num, rlist[num])) 
                    #print rlist[num]        
            else:
                log.debug("State : disk_info(%s, %d) bf: rlist[%d]is %s " % (infotype, num, num, rlist[num]))            
                for j in range (8,16):        
                    #print "(%d, %d)" % (num,j)
                    #log.debug("State : num %d cnt %d %s" % (num,j, slot_info[num][j][infotype]))
                    #print slot_info[num][j][infotype]
                    rlist[num].append(slot_info[num][j][infotype])
                    #log.debug("State : disk_info(%s, %d) rlist[%d] is %s" % (infotype, num, num, rlist[num]))            
                    #print rlist[num]        
        else:
            log.info("State : no disks in slot %d" % (i))

        return rlist
################################################################
# config_remove, erases the mark6 config.xml file before rebooting or restarting application        
def config_remove():
    global config
    rv = 0 # --- ACTION SUCCESS

    # --- REMOVE STATE  CONFIGURATION INFO
    if(os.path.isfile(config)):
        os.remove(State.config)
        log.info("Mark6 Config file removed")
    else:
        log.warning("Mark6 Config file did not exist")         
        rv = 1   # --- CONFIG FILE DID NOT EXIST

    return rv

################################################################
# save_state - saves the input_streams, disk_module group states to
# config.xml file before rebooting or restarting application        
def save_state():
    global config
    rv = 1 # --- config file not saved

    return rv 
    
