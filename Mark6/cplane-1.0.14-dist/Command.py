# Copyright 2011 MIT Haystack Observatory
##
## This file is part of Mark6 VDAS.
##
## Mark6 VDAS is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 2 of the License.
##
## Mark6 VDAS is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Mark6 VDAS.  If not, see <http://www.gnu.org/licenses/>.

'''
Original Author:   del@haystack.mit.edu
Author / Maintainer: Chet Ruszczyk chester@haystack.mit.edu
Date:     5/12/2011
Description:

Command.py is a set of Classes representing setting valid commands
and valid queries.  All of the classes are of the form NameCommand
or NameQuery.  Once completed a response is generated.

'''

import os
import Response
import State
import logging
import Utils 
import time
import module_libs
import subprocess
import m6sc
from sys import exit
from getpass import getuser
'''
These commands are match version 3.0.2 of the Mark6 command set
'''
        
COMMANDS = set([
    'input_stream',
    'mod_init',
    'record',
    'group',
    'group_members',
    'gsm',
    'gsm_mask',
    'disk_info',
    'msg',
    'mstat',
    'rtime',
    'scan_check',
    'scan_info',
    'status',
    'sys_info',
    'delete',
    'execute',
    'list',
    'm6cc',
    'dterm',
    'cterm',
    'dts_id',
    'test',
    ])

log = logging.getLogger('M6')


class Command(object):

    '''
    Parse incoming VSIS commands.
    '''

    def __init__(self, parsed=None):
        params = parsed['params']
        if State.check_disks:
            log.info("check the disk modules")
            State.check_disks = 0

        for p, P in zip(params, self.__class__.PARAMS):
            setattr(self, P, p)

    def __getitem__(self, key):
        return getattr(self, key, None)

    def todict(self):
        return dict([(x, getattr(self, x)) for x in self.__class__.PARAMS])

    def todict_generator(self):
        for x in self.__class__.PARAMS:
            yield (x, getattr(self, x))

    def __iter__(self):
        return self.todict_generator()

    def execute(self):
        raise Exception('Please implement me.')


class Query(Command):
    pass

################### disk_info? ##########################################
class DiskInfoQuery(Query):
    PARAMS = [
        'info_type',
        'vol_ref',
        ]

    def execute(self):
        
        #print "Query = ", Query
        #log.debug('Command:DiskInfoQuerys: %s, %s'% (self.info_type, self.vol_ref))
        l = []

        # --- IF A PARAMETER WAS NOT PASSED, AN ATTRIBUTE ERROR IS RAISED, ASSIGN THE DEFAULT USAGE
        try:
            if self.info_type != None:
                it = self.info_type

        except AttributeError:
            it = 'usage'


        try:
            if self.vol_ref != None:
                vf  = int(self.vol_ref)

        except AttributeError:
            vf = -1



        if it in ['serial', 'model', 'vendor', 'usage', 'size', 'temp']:
            log.debug("Command - info_type is %s for disk_module %d" % (it,vf))
            msn_list = State.module_list()
            log.debug("Command - returned State.module_list %d ", msn_list)

            if msn_list > 0:
                # --- CHET HERE HAVE THE LIST NEED TO CALL DISKINFO 
                log.debug('Command - Getting information about modules ')
                #print msn_list keys are 
                # --- disk controller 0
                dc_0 = State.msn_state.keys()
                log.debug("Command - msn_state keys are %s " % (dc_0))
                msn_dict = State.module_info(it, vf)
                log.debug("Command msn_dict is %s" % (msn_dict))

                # --- REQUIRE COUNTER SO LISTS ARE 0,1,...
                cnt = 0
                for i in msn_dict.keys():
                    nd = str(len(msn_dict[i]))
                    log.debug("msn_dict processing for host %d that has %s disks" % (i, nd))
                    l.append([str(i), State.slot_status[i]['eMSN'] ,str(State.slot_status[i]['disks']) , str(State.slot_status[i]['rdisks']),nd])
                    #print "l=", l
                    #print msn_dict[i]

                    for j in msn_dict[i]:
                        log.debug("Command - appending %s to list " % j) 
                        l[cnt].append(j)

                    log.debug("Command - final list to return is %s" % l[cnt])

                    cnt += 1

            r = Response.DiskInfo(return_code='0',
                                  cplane_return_code='0',
                                  type=it,
                                  list=l)
        else:
            r = Response.DiskInfo(return_code='3',
                                  cplane_return_code='0',
                                  type='invalid request',
                                  list=l)
    
        return str(r)

########################## msg? #####################################
class MsgQuery(Query):

    PARAMS = [
        'error_msg',
        ]

    def execute(self):
        # --- DETERMINE THE LENGTH OF THE ERROR QUEUE
        numerrs = len(State.err_msg_queue)

        # --- IF IT IS 0, THEN NO ERROR MESSAGE IN QUEUE
        if numerrs == 0:
            r = Response.Msg(return_code='0',
                             cplane_return_code='0',
                             error_msg = 'NA',
                             list = [])

        # --- ELSE THERE ARE ERRORS, CREATE A LIST AND RESPOND
        else:
            msg = ""
            log.debug ("Command - return error_msg %s" %  State.err_msg_queue)
            for i in range(numerrs):
                msg += State.err_msg_queue.pop(0)
            
            # --- REMOVE THE LAST DELIMINATOR
            msg  = msg[:msg.rfind('|')]
            # --- CLEAR THE ERROR MSG status bit
            State.c_status &=0xffffffd

            # --- CLEAR THE ERROR BIT
            r = Response.Msg(return_code='0',
                             cplane_return_code='0',
                             error_msg = msg,
                             list=[])

        return str(r)

######################## input_stream= ##########################
class InputStreamCommand(Command):
    PARAMS = [
        'action',
        'stream_label',
        'data_format',
        'payload_size',
        'payload_offset',
        'psn_offset',
        'interface_id',
        'filter_address',
        'port',
        'subgrp_ref',
        ]

    def execute(self):
        retcode = '0'
        l = []
        # --- PARAMETER ERROR
        p_error = 0

        # --- LOWER CASE ACTION COMMAND
        action = self.action.lower()

        if action in ['add', 'delete']:
            # --- PARAMETER CHECKING STREAM_LABEL MAX LENGTH 16 CHARS
            try:
                if self.stream_label != None:
                    stream_label = self.stream_label
                    lsl = len(stream_label)
                    if lsl == 0 or lsl > 16:
                        # --- set parameter error flag
                        p_error = 1
                        msg  = "Command - Parameter error input_stream: 0 < stream_label <= 16 char, recieved %d" % lsl
                        State.add_error_msg(msg)

            except AttributeError:
                stream_label = ''
                msg = "Command - Parameter error input_stream: no stream_label provided"
                State.add_error_msg(msg)
                p_error = 1

        if action in ['add']:
            # --- PARAMETER CHECKING DATA_FORMAT IS VDIF OR M5B
            try:
                if self.data_format != None:
                    data_format = self.data_format
                    data_format.lower()
                    if data_format not in ['vdif', 'm5b']:
                        if data_format == '':
                            # --- SET TO THE DEFAULT
                            data_format = 'vdif'
                            log.debug("Command - data_format offset set to default %s" % (data_format))
                        else:
                            # --- set parameter error flag
                            msg = "Command - Parameter error input_stream: data_format invalid %s" % (data_format)
                            State.add_error_msg(msg)
                            p_error = 1
                    else:
                        log.debug("input_stream: valid data_format - %s" % (data_format))

            except AttributeError:
                # --- SET TO THE DEFAULT
                data_format = 'vdif'
                log.debug("Command - data_format offset set to default %s" % (data_format))

            # --- PARAMETER CHECKING PAYLOAD_OFFSET 0<= X < 256 
            try:
                if self.payload_offset != None:
                    payload_offset = self.payload_offset
                    if payload_offset == '':
                        payload_offset = 42
                        log.debug("Command - payload offset set to default %d" % (payload_offset))   
                    else:
                        payload_offset = int(payload_offset)
                        if (payload_offset < 0) and (payload_offset > 256): 
                            # --- set parameter error flag
                            msg = "Command - Parameter error input_stream: payload_offset invalid %d" % (payload_offset)
                            State.add_error_msg(msg)
                            p_error = 1             

            except AttributeError:
                payload_offset = 42
                log.debug("Command - payload offset set to default %d" % (payload_offset))

            # --- PARAMETER CHECKING PAYLOAD_SIZE 64 <= X <= 9000
            try:
                if self.payload_size != None:
                    if self.payload_size == '':
                        payload_size = 8224
                        log.debug("Command - payload offset set to default %d" % (payload_offset))                                                
                    else:
                        payload_size = int(self.payload_size)
                        if (payload_size >= 64) and (payload_size <= 9000): 
                            if payload_size % 8:
                                msg = "Command - Parameter error input_stream: payload_size not modulo 8 %d" % (payload_size)
                                State.add_error_msg(msg)
                                p_error = 1 
                            else:
                                log.debug("Command - Parameter input_stream: payload_size is modulo 8 " )
                        else:
                            msg = "Command - Parameter error input_stream: payload_size is valid %d" % (payload_size)
                            State.add_error_msg(msg)
                        
            except AttributeError:
                payload_size = 8224
                log.debug("Command - payload offset set to default %d" % (payload_offset))

            # --- PARAMETER CHECKING PSN_OFFSET 0<= X < 900 0
            try:
                if self.psn_offset != None:
                    if self.psn_offset == '':
                        # --- set parameter error flag
                        msg = "Command - Parameter error input_stream: psn_offset invalid none given"
                        State.add_error_msg(msg)
                        p_error = 1                                      
                    else:
                        psn_offset = int(self.psn_offset)
                        if (psn_offset >= 0) and (psn_offset <= 9000): 
                            # --- I ASSUME THAT PSN OFFSET OF 0 INDICATES NO PSN CHECKING SINCE THE MAC HEADER IS 
                            #     ALWAYS FIRST, SO SET OFFSET TO -1 FOR DPLANE
                            if psn_offset == 0:
                                psn_offset = -1
                                log.warning("Disabling psn checking, psn value of 0 was provided")
                            else:
                                log.debug("Command - psn_offset set to %d" % (psn_offset)) 
                            
                        else:
                            # --- set parameter error flag
                            msg = "Command - Parameter error input_stream: psn_offset invalid %d" % (psn_offset)
                            State.add_error_msg(msg)
                            p_error = 1              

            except AttributeError:
                psn_offset = -1
                log.infp("Command - psn_offset set to default %d, assumed disabled state" % (psn_offset))

            # --- PARAMETER CHECKING PSN_OFFSET 0<= X < 9000 
            try:
                if self.interface_id != None:
                    interface_id = self.interface_id.lower()
                    if len(interface_id) > 16 or interface_id == '':
                        # --- set parameter error flag
                        p_error = 1
                        msg = "Command - Parameter error interface_id invalid"
                        State.add_error_msg(msg)
                    else:
                        #--- SHOULD CHECK IF THE INTERFACE IS UP, IF NOT WARNING
                        log.debug("Command - interface_id being checked for state")

            except AttributeError:
                interface_id = ''
                # --- set parameter error flag
                p_error = 1
                msg = "Command - Parameter error no interface_id"
                State.add_error_msg(msg)

            # --- PARAMETER CHECKING FILTER_ADDRESS 
            try:
                if self.filter_address != None:
                    if self.filter_address == '':
                        filter_address = '127.0.0.1'
                        log.debug("Command - filter_address set to default %s" % (filter_address))
                    else:
                        filter_address = self.filter_address

            except AttributeError:
                filter_address = '127.0.0.1'
                log.debug("Command - filter_address set to default %s" % (filter_address))

            # --- PARAMETER CHECKING PORT
            try:
                if self.port != None:
                    if self.port == '':
                        port = 12000
                    else:
                        port = int(self.port)
                     
            except AttributeError:
                port = 12000
            
            log.debug("Command - port set to %d" % (port))

            # --- PARAMETER CHECKING PORT
            try:
                if self.subgrp_ref != None:
                    print "subgrp_ref provided  %s " % self.subgrp_ref
            
                    if self.subgrp_ref == '':
                        # --- IF THE SUBGRP_REF PASSED IS WHITESPACE ASSIGN 0 WHICH REPRESENTS THE OPEN GROUP.
                        state.subgroup_ref[0] = '0'
                        log.debug("Command - subgrp_ref set to default %s" % (subgrp))
                    else:
                        if self.subgrp_ref.isdigit() :
                            subgrp = self.subgrp_ref
                            #if subgrp < 1 and subgrp > 4:
                            #    msg = "invalid subgroup_ref %s provided for input_stream command must be {1..4}" % self.subgrp_ref
                            #    State.add_error_msg(msg)
                            #    # --- set parameter error flag
                            #    p_error = 1                   
                            #else:
                            log.info("input_stream %s data flow to be directed only to modules %s" % (interface_id, subgrp))
                            #State.subgroup_ref
                                
                        else:
                            msg = "invalid subgroup_ref %s provided for input_stream command must be {1..4}" % self.subgrp_ref
                            State.add_error_msg(msg)
                            # --- set parameter error flag
                            p_error = 1                        
                    
            except AttributeError:
                subgrp = '0'
                log.debug("Command - subgrp set to default entire open group %s" % (subgrp))

        log.debug("Command:InputStreamCommand - action to be completed is %s" % action)

        if p_error == 0:
            if action == 'add':
                retcode = str(State.add_input_stream(stream_label,
                                                     data_format,
                                                     payload_offset,
                                                     payload_size,
                                                     psn_offset,
                                                     interface_id,
                                                     filter_address,
                                                     port,
                                                     subgrp))
            elif action == 'delete':
                retcode = str(State.delete_input_stream(stream_label))

            elif action == 'commit':
                # --- FORMAT AND SEND DATA TO D-PLANE
                retcode = str(State.commit_input_stream())

                # --- UDPATE XML CONFIGURATION INFORMATION FOR PERSISTENCY
                # State.is_commit()

                # --- UPDATE STATUS THAT THE INPUT STREAM HAS BEEN COMMITTED AND READY TO RECEIVE DATA
                State.c_status |= 0x200

            else:
                msg = 'Command:InputStreamCommand - Invalid input_stream action : %s' % action
                State.add_error_msg(msg)
                retcode = '3'
        else:
            msg = 'Command:InputStreamCommand - Invalid input_stream for action : %s' % action
            State.add_error_msg(msg)
            retcode = '3'

        log.debug("Command - return code is %s" % retcode)
        r = Response.SetInputStream(return_code=retcode, cplane_return_code='0', list=l)
        log.debug("Command - returned %s" % r)
        return str(r)

###################### input_stream? ###################################
class InputStreamQuery(Query):
#    PARAMS = [
#        'action',
#        'stream_label',
#        'data_format',
#        'interface_ID',
#        'filter_address',
#        ]
    
    PARAMS = [
        'stream_label',
        ]

    def execute(self):
        log.debug("Command:InputStreamQuery Enter" )

        s = State.get_input_stream(getattr(self, 'stream_label', None))
        ls = len(s)
        log.debug( "Command - the number of input streams is %d " % ls)
        l = []
        
        log.debug("Command defined input streams are %s " % s)
        if ls > 0:
            for e in s:
                #print e
                if e != None:
                    #print "append data"
                    l.append([e['stream_label'],
                              e['data_format'],
                              str(e['payload_size']),
                              str(e['payload_offset']),
                              str(e['psn_offset']),
                              e['interface_id'],
                              e['filter_address'],
                              str(e['port']),
                              e['subgroup']])

            log.debug("Command - calling Response.getinputstream e!=None()")
            r = Response.GetInputStream(
                return_code='0', cplane_return_code='0', list=l)
            #print "r = ", r
        #else:
        #    print "calling Response.getinputstream e==None()"
        #    r = Response.GetInputStream(
        #        return_code='1', cplane_return_code='3', list=l)
        #    print "r = ", r
        else:
            r = Response.GetInputStream(return_code='0', cplane_return_code='0', list=l)
            
        #print "returning with ", r
        return str(r)

############################# mod_init= ###############################
class ModInitCommand(Command):
    PARAMS = [
        'slot',
        'number_disks',        
        'msn',
        'type',
        'new',
        ]

    def execute(self):
        l = []   # ---- RETURN LIST MSN AND NUMBER OF DISKS
        slot = self.slot
        nd = self.number_disks
        valid_params = 0     # --- assume parameters passed are in valid before checking

        # --- if check_disks flag is set, update disk module information before continuing
        if State.check_disks:
            log.debug("Command - should check the disk module state before processing command ")
            

        if slot.isdigit():
            islot = int(slot)
            # --- IF THE SLOT NUMBER TO INITIALIZE IS VALID
            if islot in range(1,9):
                log.debug("Command - slot is  %d " % islot)
                # --- IS THE SLOT TO INITIALIZED MODULE BEEN PROCESSED, OR UNASSIGNED?
                #print "Unassigned_slots is ", State.unassigned_slots
                # --- unassinged contains strings, use slot vs. integer slot
                if slot in State.unassigned_slots:
                    log.info("Command: Slot number %d has power and has not been assigned " % islot)
                    valid_params += 1
                else:
                    log.warning("Command: There is no powered on disk module in the slot requested %d " % islot)

                if nd.isdigit():
                    ind = int(nd)
                    # --- IF THE NUMBER TO DISKS TO INITIALIZE IS VALID
                    if ind in range(1,16):
                        log.debug("Command - number of disks to initialize is %d " % ind)
                        # --- CHECK IF IT IS EQUAL TO THE NUMBER OF DISKS AVAILABLE IN THE SLOT

                        # --- ONLY IF IT MATCHES THEN NUMBER OF DISKS AVAILABLE, INCREMENT VALID_PARAMS
                        valid_params += 1

                        # --- MSN PROVIDED AND 8 CHARACTERS?
                        if len(self.msn) == 8:
                            msn = self.msn
                            log.debug("Command - MSN is valid length %s " % msn )
                            valid_params += 1

                            # --- THE FILE NAME HAS TO BE PROVIDED
                            try:
                                if self.type != None:
                                    # --- set the file type to initialize to scatter / gather mode
                                    ft = self.type
                                    if ft in ('sg', 'raid'):
                                        log.debug("Command - type of module is valid %s " % ft )
                                        valid_params += 1

                                    else:
                                        log.warning("Command - %s is not a valid type " % ft )
                            except AttributeError:
                                ft = "sg"
                                valid_params += 1
                        
                                log.debug("Command - setting type of module to %s " % ft )

                            # --- THE FILE NAME HAS TO BE PROVIDED
                            try:
                                if self.new != None:
                                    # --- set the file type to initialize to scatter / gather mode
                                    new = self.new
                                    if new == 'new':
                                        log.debug("Command - new field is valid %s " % new )
                                        valid_params += 1
                                    elif new == '' or new == 'null':
                                        log.debug("Command - new field is valid blank setting to NULL " )
                                        new = "null"
                                        valid_params += 1
                                    else:
                                        log.warning("Command - -%s- is not a valid field " % new )
                            except AttributeError:
                                new = "null"
                                valid_params += 1
                        
                                log.debug("Command - setting type of module to %s " % new )

                        else:
                            log.info ("Command: MSN is NOT a 8 characters  -%s- " % self.msn )
                    else:
                        log.info("Command:  number of disks to initialize is not valid value {1-16} but %d" % ind)
                else:
                    log.info("Command: slot is not in allowed range (1-9) but %s" % nd)

        log.debug("Command: number of valid parameters is %d" % valid_params)

        # --- DID WE RECEIVE THE PROPER NUMBER OF PARAMETERS?
        if valid_params == 5:
            
            # --- SET PROCESS TO INITIALIZE DISK THIS WILL TAKE A WHILE.
            log.warning("Command: Module Initialization may take awhile ")
            # --- VERIFY THE MODULE STATUS IS NOT OPEN
            if not State.slot_status[islot]['status1'] in ['open', 'closed']:
                log.debug("Command : the modules status is not open or closed, proceed")
                
                if ft == "sg":
                    #print "ft is ", ft
                    # --- (CHET) - SHOULD CHECK IF THEY ARE SG, AND IF NEW == NULL, THEM REMOVE GROUP AND
                    #     ERASE THE DISK, WHOULD IT BE EASIER JUST TO RECREATE THE MODULE, THAT REMOVES ALL DATA?

                    # --- MAKE SURE THE DISKS ARE NOT MOUNTED
                    if new == "new":
                        if module_libs.sg_unmount(islot):
                            (rc, disks_conf, cap) = module_libs.sg_init(islot, msn, ind )
                            log.info("module-libs : for slot %d sg_init returned capacity of %s and #disk %d" % (islot, cap, disks_conf))      
                        else:
                            msg = "mod_init for slot %d failed, disk drives could not be unmounted" % (islot)
                            State.add_error_msg(msg)
                            rc = '4'
                            # --- ADD A D6 RETURN CODE, FOR DISK MOUNTED FOR FORMATTING OPERATION
                    # --- THEN IT IS A NULL, UPDATE SG INFO
                    else:
                        #print "calling remove_group"
                        # --- (CHET) NEED TO CHECK IF MSN PROVIDED MATCHES LABEL OF DISK, AND MOUNTED
                        rv = module_libs.remove_group(islot, ft)
                        # --- if success then update status
                        if rv:
                            log.info("module-libs : for slot %d sg_updated" % (islot))      
                            rc = '0'
                            cap = State.slot_status[islot]['cap']
                            #print 'cap ', cap
                            disks_conf = State.slot_status[islot]['rdisks']
                            #print disks_conf
                        else: rc = '1'

                else:
                    # --- MAKE SURE THE DISKS ARE NOT MOUNTED
                    if module_libs.raid_mnt('unmount', islot):
                        (rc, disks_conf, cap) = module_libs.raid_zero_init(islot, msn, ind)
                    else:
                        msg = "mod_init for slot %d failed, disk drives could not be unmounted" % (islot)
                        State.add_error_msg(msg)
                        rc = '4'
                        # --- ADD A D6 RETURN CODE, FOR DISK MOUNTED FOR FORMATTING OPERATION
            else:
                log.warning("Command : the slot to mod_init has a status of %s cannot initialize" % State.slot_status[islot]['status1'])
                rc = '6'

            # --- IF RETURN WAS A SUCCESS UPDATE SLOT_STATUS FOR INITIALIZED DISKS
            if rc == '0':

                eMSN = module_libs.set_eMSN(msn, islot, cap, 4, disks_conf, ft)
                log.debug("Command : eMSN being set to %s for slot %d" % (eMSN, islot))
                State.slot_status[islot]['eMSN'] = eMSN
                State.slot_status[islot]['disks'] = ind
                State.slot_status[islot]['rdisks'] = disks_conf
                State.slot_status[islot]['cap'] = cap
                State.slot_status[islot]['type'] = ft
                State.slot_status[islot]['group'] = "-"     # --- RESET THE GROUP STATUS"
                State.slot_status[islot]['status1'] = "initialized"     # --- RESET THE GROUP status1
                State.slot_status[islot]['status2'] = "null"     # --- RESET THE GROUP status1

                # --- UPDATE THE EMSN AND SERIAL NUMBER FOR THE META-DATA FOR SG MODE
                # --- RAID INITIALIZED DEVICES ARE UPDATED WHEN A NEW GROUP IS CREATED
                if ft == "sg":
                    log.debug("Command : updating the meta-data on sg module in slot %d" % islot)
                    if module_libs.update_sg(islot):
                        log.warning("Command : The update of the meta-data on sg module in slot %d was not a success" % islot)
                    else:
                        log.info("Command : The update of the meta-data on sg module in slot %d was successful" % islot)

        else:
            msg = "Command: invalid parameter send"
            State.add_error_msg(msg)
            rc = '3'        

        #cmd = 'lshw -class disk -short'
        #result = subprocess.Popen(cmd)
        #result.pop(0)
        #result.pop(0)
        #disks = []
        #for r in result:
        #    d = r.split()
        #    if d[1] == 'sda':
        #        continue
        #    disks.append({ 'device': d[1], 'size': d[3]})

        r = Response.SetModInit(return_code=rc, cplane_return_code='0', list=l)

        return str(r)

    
############################# mod_init? ###############################    
class ModInitQuery(Command):
    PARAMS = [
        ]
    def execute(self):    
        l = []
        log.debug("Command:ModInitQuery - Entered calling module_list ")
        msn_list = State.module_list()
        log.debug("Command:ModInitQuery - returned from State.module_list with %d" % (msn_list))
        
        if msn_list > 0:
            log.debug('Command:ModInitQuery: Getting information about modules ')
            msn_dict = State.module_info('usage', -1 )
            log.debug("Command - msn_dict is %s" % msn_dict)
        
            log.debug("Command:ModInitQuery: returned module_info()")
            for i in msn_dict.keys():
                log.debug("msn_dict processing for host %d" % i)
                if State.slot_status[i]['disks'] == 0:
                    if len(msn_dict[i]) > 0:
                        State.slot_status[i]['disks'] = len(msn_dict[i])
                
                l.append([str(i),
                          str(State.slot_status[i]['disks']), # --- STORED AS INT
                          State.slot_status[i]['eMSN'],
                          ])

                log.debug('ModInitQuery interface %s, eMSN %s, #Disks %s ' % (str(i),State.slot_status[i]['eMSN'] ,State.slot_status[i]['disks']))
                    
        r = Response.GetModInit(return_code='0', cplane_return_code='2', list=l)

        return str(r)
############################### record= #########################################
class RecordCommand(Command):
    PARAMS = [
        'action',
        'duration',
        'data_size',
        'scan_name',
        'experiment_name',
        'station_code',
        ]

    def execute(self):
        log.info('RecordCommand with action: %s'% self.action)

        ######## ACTION
        if self.action != "off":
            # --- THE FILE NAME HAS TO BE PROVIDED
            try:
                if self.scan_name != None:
                    scan_name = self.scan_name
                    if scan_name == '':
                        scan_name="NULL"

            except AttributeError:
                scan_name = "NULL"

            if scan_name == "NULL": 
                scan_name = time.strftime('%Y%j%H%M%S',time.gmtime())
                scan_name = "scan_" + scan_name

            log.info("Command:RecordCommand scan_name is  %s " % scan_name)

            #### EXPERIEMENT NAME
            try:
                if self.experiment_name != None:
                    State.exp_name = exp_name = self.experiment_name
                    if exp_name == '':
                        exp_name = State.exp_name
            except AttributeError:
                exp_name = State.exp_name
            
            log.info("Command:RecordCommand exp_name is  %s " % exp_name)
            #### STATION CODE 
            try:
                if self.station_code != None:
                    State.st_code = st_code = self.station_code
                    if st_code == '':
                        st_code = State.st_code
            except AttributeError:
                st_code = State.st_code

            log.info("Command:RecordCommand station_code is  %s " % st_code)
            #### DATA_SIZE
            try:
                if self.data_size != None:
                    if self.data_size.isalnum():
                        data_size = int(self.data_size)
                        if data_size < 1:
                            # --- NEED TO MAKE THIS GLOBAL based on rjc hardcoded values
                            data_size = 8224+42 
                            log.debug("Command: RecordCommand data size is not set, setting to  %d" % data_size)
                        else:
                            data_size = int(self.data_size) + 42
            except AttributeError:
                data_size = 8224+42

            log.info("Command: RecordCommand data size is %d" % data_size)

            #### DURATION 
            try:
                if self.duration != None:
                    if self.duration.isalnum():
                        duration = int(self.duration)
                        if duration < 1:
                            # --- NEED TO MAKE THIS GLOBAL based on rjc hardcoded values
                            duration = 20
                            log.debug("Command: RecordCommand duration is <1  setting to  %d" % duration)

            except AttributeError:
                # --- SET THE DURATION TO 24 HOURS
                duration = 24*3600
            
            log.info("Command: RecordCommand setting duration to %d" % duration)
            
            # --- GENERATE A TMP SCAN NAME TO SEE IF IT IS IN THE LIST
            tmp_sn = exp_name + "_" + st_code +  "_" + scan_name
            log.debug("Command:RecordCommand full scan_name is  %s " % tmp_sn)
            #print "fn_dict ", State.fn_dict
            while State.fn_dict.has_key(tmp_sn):
                log.debug("Command:RecordCommand full scan_name exist  %s, adjusting " % tmp_sn)
                scan_name = scan_name + "a"
                
        # --- NEED TO VALIDATE ACTION INCASE IT IS VEX TIME
        if self.action == 'on':
            log.debug("Command: RecordCommand Action is ON")
            # --- get the present time in GMT
            #ts_secs = time.mktime(time.gmtime())
            ts_secs = time.time()
            start_time=ts_secs
            vextime = time.gmtime()
            log.debug("Command: RecordCommand setting ts to %d" % ts_secs)

            if type(duration) != 'int':
                duration = int(duration)
            log.debug("Command: RecordCommand setting duration to %d" % duration)

            log.debug("Command: RecordCommand data size is %d" % data_size)

            # --- if a group is open and ready, else error:            
            if State.record_session['action'] != 'on' :
                if State.open_group['slot'] != '-':
                    log.debug("Command : not recording and open_group slot is %s " % State.open_group['slot'])
                    rv = State.on_record_session(self.action,
                                                 vextime,
                                                 start_time,   
                                                 duration,
                                                 data_size,
                                                 scan_name,
                                                 exp_name,
                                                 st_code)
                else:
                    msg = "There is no open group for recording"
                    State.add_error_msg(msg)
                    rv = '4'
            else:
                msg = "The system is already in record mode with %s" % State.record_session['scan_name']
                State.add_error_msg(msg)
                rv = '4'

        elif self.action == 'off':
            log.debug("Command: RecordCommand Action is OFF")
            rv = State.off_record_session()

        else:
            # --- ASSUME THE TIME IS VALID, ONLY IF PROVEN INVALID CHANGE STATE
            valid = 1
            log.debug("Command: RecordCommand Action is assumed to be TIME")
                # --- VALIDATE INFORMATION PASSED
            #print "start_time type is ", type(self.action)
            log.debug("start_time is -%s-" % self.action)

            if len(self.action) < 1:

                # --- get the present time in GMT
                ts_secs = time.mktime(time.gmtime())
                self.start_time = time.gmtime()
                log.debug("Command: RecordCommand setting ts to ")
            else:
                # --- VALIDATE THE TIME PASSED
                log.debug("Command: RecordCommand using ts")
                vextime = self.action
                if vextime != "0":
                    (valid, start_time) = Utils.vextime_to_standard(self.action)
                    if valid:
                        log.debug("Command: RecordCommand time is converted " % start_time)
                    else:
                        log.warning("Command: RecordCommand error in parameters passed %s" % vextime)
                        rv = 2
                else:
                    log.info("Command: RecordCommand vextime was 0, leaving it")
                    valid = 1

            if valid == 1:
                    
                if len(self.duration) < 1:
                #if self.duration is None:
                    self.duration = 3600
                    log.info("Command: RecordCommand setting duration to default 3600")
                else:
                    if type(self.duration) != 'int':
                        self.duration = int(self.duration)

                if scan_name == "NULL": scan_name = vextime


                log.debug("Command: RecordCommand setting duration to %d" % self.duration)
                if State.record_session['action'] != 'on' or State.record_session['action'] != 'time':
                    if State.open_group['slot'] != '-':      
                        rv = State.on_record_session(self.action,
                                                     vextime,
                                                     start_time,   
                                                     duration,
                                                     data_size,
                                                     scan_name,
                                                     exp_name,
                                                     st_code)
                        # --- AFTER COMMAND HAS BEEN SENT SET ACTION AS TIME.
                        State.record_session['action'] = 'time'
                    else:
                        msg = "There is no open group for recording"
                        State.add_error_msg(msg)
                        rv = '4'
                else:
                    msg = "The system is already in record mode with %s" % State.record_session['scan_name']
                    State.add_error_msg(msg)
                    rv = '4'

            else:
                msg = 'Invalid record action'
                rv = '2'
        
        srv = str(rv)
        # TODO: fill in parameters.
        r = Response.SetRecord(return_code=srv, cplane_return_code=srv, list=[])

        return str(r)

############################### record? #########################################
class RecordQuery(Query):
 
    PARAMS = [
        'status',
        'scan_number',
        'scan_name',
        ]


    def execute(self):
        log.info('RecordQuery: Enter')
        scan_num = "0"

        rs = State.get_record_session()   
        log.debug('RecordQuery: record_session state is %s', rs)
        if rs['action'] == 'on':
            # --- NEED TO USE THREAD RECEIVING STATUS FROM D-PLANE FOR THIS, FOR NOW STATE RECORDING
            state = 'recording'
        elif rs['action'] == "time":
            # --- NEED TO CHECK OF STARTTIME HAS PASSED, 
            state = 'pending'
        elif rs['action'] == "off":
            # --- NEED TO SEE IF BUFFERS ARE BEING FLUSHED, FOR NOW, SET TO OFF
            state = "off"
        elif rs['action'] == "none":
            state = "off"
            #scan_num = "0"
            rs['scan_name'] = "-"
        else:
            state = "UNK"
            scan_num = "x"
            rs['scan_name'] = '-'

        if scan_num == "0":
            scan_num = str(len(State.file_dict.keys()))
            

        r = Response.GetRecord(return_code='0', cplane_return_code='0',
                               status=state,
                               scan_number=scan_num,
                               scan_name=rs['scan_name'],
                               list = [])

        return str(r)


#########################  rtime? ##########################################
class RtimeQuery(Query):
    PARAMS = [
        'data_rate',
        ]
    
    def execute(self):
        log.debug('RtimeQuery Entered:')
        l = []
        # --- INITIALIZE RETURN VARIABLES
        DataRate = rt = GB_remain = GB_total = 0
        group = '-'

        try:
            if self.data_rate != None:
               DataRate = int(self.data_rate)
        except AttributeError:
            DataRate = 4*4
        
        # --- IF THERE IS AN OPEN GROUP, GET VALUES
        if State.open_group['slot'] != '-':
            # --- FOR THE OPEN GROUP, ASSUME NOT ASSIGNED RIGHT NOW
            group = State.open_group['slot']
            # --- CALL UTIL.SPACE() TO GET RESPONSE INFORMATION
            (rv, rt, GB_remain, GB_total) = Utils.space(group, DataRate)
            log.debug("Command : space returned with %d, %d, %d" % (rt, GB_remain, GB_total))

        # --- INFORMATION TO PASSED TO RESPONSE
        ''' 'group',
        'data_rate',
        'remaining_time',
        'remaining_GB',
        'available_GB',    
        '''
        l.append([group,
                  str(DataRate),
                  str(rt),
                  str(GB_remain),
                  str(GB_total)])
        # --- NEED TO START FILLING THIS SECTION OUT. (CHET)
        r = Response.GetRtime(return_code='0',
                              cplane_return_code='0',
                              list = l)

        log.debug("RtimeQuery Exit with %s" %r)

        return str(r)    

class RtimeCommand(Command):
    PARAMS = [
        'data_rate',
        ]
    
    def execute(self):
        l = []
        log.debug('RtimeCommand Entered:')
        # --- NOT SUPPORTED. 
        r = Response.SetRtime(return_code='2',
                              cplane_return_code='Set not supported for rtime',
                              list =l)
        log.debug("RtimeCommand Exit with %s" %r)

        return str(r)    

######################### end rtime ##############################################

######################### scan_check? ##############################################
class ScanCheckQuery(Query):
    PARAMS = [
        'scan_name',
        'group_ref',
        ]

    def execute(self):
        rc = 0
        verify_sname = 0
        d6_rc = 0
        sn = 0            # --- initialize scan number to zero
        l = []
        ns = 'unk'        # --- initialize number of streams to unknown
                
        log.debug('ScanCheckQuery Entered:') 
        
        try:
           # print "self.scan_name is: "
           # print self.scan_name
            if self.scan_name != None:
                if self.scan_name.isdigit():
                    sn = int(self.scan_name)
                    #print "scan number ", sn
                    #print "keys of file_dict are: ", State.file_dict.keys()
                    if sn in State.file_dict:
                        #print "file_dict haskey ", sn
                        sname = State.file_dict[sn]['sn']
                        ns = str(State.file_dict[sn]['num_str'])
                        #print "scan name = ", sname
                        rc = 1
                    else:
                        #print "file_dict did not have key ", sn
                        rc = 3
                        log.info("scan_check, sn is not in file-dict ")
                        msg = "scan_number %d is not in list" % sn
                        State.add_err_msg(msg)
                else:
                    sname = self.scan_name 
                    #print "sname - passed ", sname
                    log.debug("Command : ScanCheckQuery - request for scan %s" % (sname))
                    verify_sname = 1

        except AttributeError:
            log.info("Command : ScanCheckQuery - using last scan number %d for query" % State.sn)
            
            rc = 1  # --- RETURN CODE SET TO PERFORM SCAN CHECK
            #print "file_dict is ", State.file_dict
            #print "record_session is ", State.record_session['action']
            if State.record_session['action'] in ['off', 'none']:
                sn = State.sn    # --- INITIALIZE SCAN NUMBER TO LAST RECORDED SCAN
                sname = State.file_dict[sn]['sn']  # --- INITIALIZE SCAN NAME TO LAST RECORD SCAN
                ns = str(State.file_dict[sn]['num_str'])
                log.debug("Command : ScanCheckQuery - Record = off, sn = %d, scan = %s, num_streams %s" % (sn, sname, ns))

            # --- ELSE IF THE SCAN NUMBER IS NOT ZERO
            elif not State.sn:
                sn = State.sn -1    # --- INITIALIZE SCAN NUMBER TO LAST RECORDED SCAN                
                sname = State.file_dict[sn]['sn']  # --- INITIALIZE SCAN NAME TO LAST RECORD SCAN
                ns = str(State.file_dict[sn]['num_str'])
                log.debug("Command : ScanCheckQuery - Record is not off, sn = %d, scan = %s, num_streams %s" % (sn, sname, ns))

            else:
                rc = 6   # --- record is not off, and it is the first scan cannot execute.
                sname = '-'
                log.info("Command : ScanCheckQuery - Recording no scan_check to perform ")

        try:
            if self.grep_ref != None:
                gref = self.gref
                log.debug("Command : ScanCheckQuery - request for group_ref %s" % (gref))
                # --- IS GROUP MOUNTED OR OPEN?
                #(d6_rc, gref)  = State.group(act, param)
                #gref = str(gref)
                rc = 1

        except AttributeError:
            gref = State.open_group['slot']
            log.info("Command : ScanCheckQuery - open gref will be used %s" % gref )

        # --- WERE ALL ATTRIBUTES CORRECT, DO WE NEED TO VERIFY SCAN_NAME PASSED
        if verify_sname:
            # --- NEED TO VERIFY THE FILENAME PASSED EXISTS
            if State.fn_dict.has_key(sname):
                sn = State.fn_dict[sname]
                # --- IF THE GROUP IS OPEN, CHECK THE LISTING
                log.debug("Verifying scan name passed")
                ns = str(State.file_dict[sn]['num_str'])
            else:
                sn = 0

            # --- ASSUME IT IS VALID FOR NOW (chet)
            verify_sname = 0
            rc = 1

        if rc == 1:
            # --- MAKE THE GROUP A LIST
            lgref = list(gref)

            # --- IF THE GROUP IS SG
            if State.slot_status[int(lgref[0])]['type'] == "sg":
                dirs = State.mnt_data[:State.mnt_data.rfind('/')]
            else:
                dirs = State.raid_data
            
            log.debug("Command : ScanCheckQuery directory is %s " % (dirs))
            scs = 0

            sname_ext = sname + '.vdif'
            print "sname_ext ", sname_ext
            num_disks = 0
            for i in lgref:
                num_disks += State.slot_status[(int(i))]['rdisks']
            tmp_sname_ext = "data/"+sname_ext
            
            log.info("Command : ScanCheckQuery the number of disks to process is %d for %s" % (num_disks, tmp_sname_ext))
            m6sc.start(sname_ext, num_disks, 0, 0.0)
            #m6sc.start(tmp_sname_ext, num_disks, 0, 0.0)
            #print "returned m5sc.start"
            for m in lgref:
                im = int(m)
                for d in range(0,8):
                    ffn ='%s/%d/%d/%s/%s' % (dirs,im,d,'data',sname_ext)
                    #print 'scs = scs + m6sc.check_file(%s,1)' % file
                    log.debug('scs = scs + m6sc.check_file(%s,1)' % ffn)
                    rv = m6sc.check_file(ffn,0)
                    if rv == 0:
                        scs = scs + rv
                    else:
                        log.debug("m6sc.check_file ret FAIL for module %d, on disk %d" %(im, d))
                        rc = 4
                        d6_rc = '140'
            
            scancheck = m6sc.status(sname_ext)
            log.debug("m6sc.status returned scancheck %s "%  scancheck)
            # --- FORMAT STATUS RETURND
            #rdxxx_sh_test21.vdif:data?:vdif:2014y073d14h56m49s:19.914:5.118:2.056:0
            m6sc_info = scancheck.split(':')
            log.debug("m6sc_info %s" % m6sc_info)
            # --- IF MISSING IS ZERO, IT IS NOT ALWAYS RETURNED, NEED TO TALK gbc
            if len(m6sc_info) ==  8:
                (fn, sref, fmt, starttime, dur, scansize, rate, missing) = m6sc_info
            else:
                (fn, sref, fmt, starttime, dur, scansize, rate) = m6sc_info                
                missing = '0'

            # --- SINCE WE ARE RETURNING WITH THE SCAN CHECK INFO, SET RC = 0, THIS WILL CHANGE LATER
            #    WHEN SCAN_CHECK= IS IMPLEMENTED (tbd)
            rc = 0
                
            l.append(['-',              # --- STREAM LABALE
                      sref,             #--- STATUS
                      fmt,
                      starttime,
                      dur,
                      scansize,
                      rate,
                      missing])
            
        else:
            rc = 3
            

        r = Response.GetScanCheck(return_code=str(rc),
                                  cplane_return_code=str(d6_rc),
                                  groupref=gref,
                                  scan_num=str(sn),
                                  scan_label=sname,
                                  num_streams=ns,
                                  list=l)
            
            

        log.debug("ScanCheckQuery Exit with %s" %r)

        return str(r)    


class ScanInfoCommand(Command):
    PARAMS = []
    
    def execute(self):
        log.debug('Command:ScanInfoCommand Entered:')
        l = []
        # --- INVALID REQUEST
        r = Response.SetScanInfo(return_code='2',
                            cplane_return_code='2',
                            list=l)

        log.debug("Command: ScanInfoCommand Exit with %s" %r)

        return str(r)    



######################### scan_info?  ##############################################
class ScanInfoQuery(Query):
    PARAMS = [
        'scan_name',
        'group_ref',
        ]

    def execute(self):
        l = []
        log.debug('ScanInfoQuery Entered:') 
        
        try:
            #print "self.scan_name is: -%s-", self.scan_name
            if self.scan_name != None:
                sn = self.scan_name 
                log.debug("Command : ScanInfoQuery - request for scan %s" % (sn))
                # --- IF THE QUERY WAS FOR A SCAN NAME NOT SCAN NUMBER FIND NUMBER
                if not sn.isdigit():
                    #print "sn %s is not a number " % sn
                    if State.fn_dict.has_key(sn):
                        log.debug("Command : ScanInfoQ - scan name %s converted to scan number %d", sn, State.fn_dict[sn])
                        sn =  State.fn_dict[sn]
                    else:
                        log.info("Command : scaninfoQ - fn_dict %s does not have a key for %s" % (State.fn_dict, sn))
                        

        except AttributeError:
            log.info("Command : ScanInfoQuery - using last scan_number for query %d" % (State.sn))
            sn = State.sn

        try:
            if self.grep_ref != None:
                gref = self.gref
                log.debug("Command : ScanInfoQuery - request for group_ref %s" % (gref))
                #(d6_rc, gref)  = State.group(act, param)
                #gref = str(gref)

        except AttributeError:
            log.info("Command : ScanInfoQuery - open gref will be used %s" % State.open_group['slot'])
            gref = State.open_group['slot']


        if sn > 0:
            #print "file_dict for %d is %s" %  (sn, State.file_dict[sn])
            if type(State.file_dict[sn]['start_tm']) == time.struct_time:
                stime = time.strftime('%Yy%jd%Hh%Mm%Ss',State.file_dict[sn]['start_tm'])
            elif (State.file_dict[sn]['start_tm'] != 'unk'):
                stime =  time.strftime('%Yy%jd%Hh%Mm%Ss',time.strptime(time.ctime(State.file_dict[sn]['start_tm'])))
            else:
                stime = time.strftime('%Yy%jd%Hh%Mm%Ss',time.strptime(State.file_dict[sn]['create_time']))

            #print "stime is", stime
            #print "type is ", type(stime)
            r = Response.GetScanInfo(return_code='0',
                                     cplane_return_code='0',
                                     groupref=str(gref),
                                     scan_number=str(sn),
                                     scan_label=State.file_dict[sn]['sn'],
                                     status=State.file_dict[sn]['status'],
                                     start_time=stime,
                                     duration=str(State.file_dict[sn]['dur']),
                                     num_data_streams=str(State.file_dict[sn]['num_str']),
                                     sys_perf_code=str(State.file_dict[sn]['spc']),
                                     list=l)
        else:
            r = Response.GetScanInfo(return_code='0',
                                     cplane_return_code='0',
                                     groupref=str(gref),
                                     scan_number=str(0),
                                     scan_label='-',
                                     status='-',
                                     start_time='-',
                                     duration='0',
                                     num_data_streams='0',
                                     sys_perf_code='0',
                                     list=l)            
        log.debug("StatusQuery Exit with %s" %r)

        return str(r)    



################# status? ##############################
class StatusQuery(Query):
    PARAMS = []

    def execute(self):
        l = []
        log.debug('StatusQuery Entered:')

        # --- IF THE DPLANE OPERATIONAL BIT IS NOT SET, CHECK IF EXECUTING 
        rv = Utils.dplane_status()

        #if not rv:
        #    msg = "Command d-plane is NOT running "
        #    State.add_error_msg(msg)            

        s_status = "0x%07x" % State.c_status

        # --- NEED TO START FILLING THIS SECTION OUT.  C_status reflects the 
        #     proper status now.  

        r = Response.Status(return_code='0',
                            cplane_return_code='0',
                            status_word=s_status,
                            list=l)
        log.debug("StatusQuery Exit with %s" %r)

        return str(r)    

################# sys_info? ###########################
class SysInfoQuery(Query):
    PARAMS = []

    def execute(self):
        log.debug('SysInfoQuery Entered:')
        l = []
        num_ports = 0
        # --- get software state information
        os_rev = '6.0.7'
        os_type_rev='Debian Squeeze'+'_'+os_rev
        log.info("Command OS revision is %s " % os_type_rev)
        log.info("Command - hardware serial number is %s " %  State.hw_serial_number )
        # --- get hardware information 
        loc = State.hw_serial_number.find("-")

        serial_num = State.hw_serial_number[loc+1:]
        log.debug("Command - serial number is %s " % (serial_num))
        # --- SET THE NUMBER OF DISKS SUPPORTED TO TWO MODULES
        num_disks = '16'

        if State.hw_exp_serial_number != "YYYY":
            loc = State.hw_serial_number.find("-")
            serial_num = serial_num + "-" + State.hw_exp_serial_number[loc:]
            log.debug("Command expansion chassis serial number is %s" % (serial_num))

            num_disks = '32'

        net_dict = State.m6_net_interfaces()
        for i in net_dict.keys():
            num_ports += 1
            l.append([i,
                      net_dict[i]['ip'],
                      net_dict[i]['state'],
                      net_dict[i]['speed']])
            log.debug('SysInfoQuery interface %s, %s, %s, %s, ' % (i, net_dict[i]['ip'],net_dict[i]['state'], net_dict[i]['speed']))

        s_num_ports=str(num_ports)
                                        
        r = Response.SysInfo(return_code='0',
                             cplane_return_code='2',
                             system_type='Mark6',
                             mark6_sn=serial_num,
                             os_type_rev=os_type_rev,
                             cplane_version_number=State.Version,
                             command_set_revision=State.CmdSet_rev,
                             available_ram=State.total_mem,
                             num_data_disks_supported=num_disks,
                             num_ethernet_input_ports=s_num_ports,
                             list=l
                            )
        log.debug("SysInfoQuery Exit with %s" %r)

        return str(r)    

################# DTS_id? ###########################
class DtsIdQuery(Query):
    PARAMS = []

    def execute(self):
        log.debug('DtsIdQuery Entered:')
        l = []
        # --- get hardware information 
        if State.hw_exp_serial_number != 'YYYY':
            hw_sn = State.hw_serial_number+'-'+State.hw_exp_serial_number
        else:
            hw_sn = State.hw_serial_number            
        
        r = Response.GetDtsId(return_code='0',
                              mark6_sn=hw_sn,
                              sw_version_number=State.Version,
                              command_set_revision=State.CmdSet_rev,
                              list=l
                              )
        log.debug("DtsIdQuery Exit with %s" %r)

        return str(r)    

##########################  group= ###################################
class GroupCommand(Command):
    PARAMS = [
        'action',
        'param',
        ]

    def execute(self):
        log.debug('Command:GroupCommand Entered:')
        l = []

        d6_rc = '0'    # --- VSI RETURN CODE
        cp_rc = '0'    # --- CPLANE RETURN CODE, GROUP = 120-129
        gref = '-'

        # --- IF A PARAMETER WAS NOT PASSED, AN ATTRIBUTE ERROR IS RAISED AND AN ERROR
        try:
            if self.action != None:
                act = self.action
                log.debug("Command : GroupCommand - has action %s" % (act))

                if act in ['auto', 'new', 'open', 'protect', 'unprotect', 'erase', 'mount', 'unmount']:
                    log.debug("Command - GroupCommand - action IS VALID")
                    
                    if act != 'auto':
                        # ---                     
                        try:
                            if self.param != None:
                                param = self.param
                                log.debug("Command : GroupCommand - has param %s" % (param))
                                
                                if param.isdigit():
                                    # --- CONVERT THE GROUP_REF TO A list 
                                    lparam = list(param)
                                    # --- SORT THE LIST IN ORDER
                                    lparam.sort()
                                    log.debug("Command : getslots returned %s " % lparam)
                                    # --- NUMBER OF SLOTS BEING GROUPED
                                    slot_cnt = 0
                                    # --- FOR ALL SLOTS IN PARAM LIST
                                    for i in lparam:
                                        ii = int(i)
                                        if ii in [1,2,3,4]:

                                            # --- VERIFY THE MODULE IS POWERED UP AND CHECK THE GROUP STATUS
                                            log.debug("Command : group is %s " % State.slot_status[ii]['group'])
                                            # --- IF THIS IS AN AUTO OR NEW, VERIFY IT IS NOT ASSIGNED.
                                            '''  (CHET) I MAY NEED TO COME UP WITH A TEST BY READING THE META DATA FROM THE 
                                                  GROUP TO SEE IF IT HAS BEEN ASSIGNED.  THINK ABOUT THIS.
                                            '''

                                            if act in ['new'] :
                                                if State.slot_status[ii]['group'] == '-':
                                                    slot_cnt += 1
                                                    log.debug("Command : slot_cnt is %d " % slot_cnt )
                                                else:
                                                    log.warning("Command : one of the group_ref %d was not unassigned -> %s " % (ii,State.slot_status[ii]['group'] ))
                                                    d6_rc = '6'

                                            elif act in ['open', 'mount']:
                                                #if  State.slot_status[ii]['group'] == '-' or i in State.slot_status[ii]['group']:
                                                
                                                if i in State.slot_status[ii]['group'] :
                                                    slot_cnt += 1
                                                    log.debug("Command : slot_cnt is %d " % slot_cnt ) 
                                                    # --- COPY A VALUD GROUP FOR COMPARISTON BEFORE DOING A GROUP
                                                    lgroup = State.slot_status[ii]['group']
                                                else:
                                                    log.warning("Command : one of the group_ref %d was illegal -> %s " % (ii,State.slot_status[ii]['group'] ))
                                                    d6_rc = '6'

                                                #print "act ", act
                                                #print "open group is ", State.open_group['slot']
                                                # --- IF TRYING TO OPEN A GROUP AND A GROUP IS ALREADY OPEN, FAIL.
                                                if act =='open' and State.open_group['slot'] != '-':
                                                    print "param type ", type(param)
                                                    print param
                                                    
                                                    msg = 'Attempting to open group %s when group %s is already open' % (param, State.open_group['slot'])
                                                    print "msg is ", msg
                                                    State.add_error_msg(msg)
                                                    d6_rc = '6'
                                                    cp_rc = '30'   # --- attempting to open a group when a group is open

                                        else:
                                            log.warning("Command : one of the group_ref was illegal, not 1,2,3,4 -> %s " % i)
                                            d6_rc = '6'

                                    # --- (CHET) SHOULD VERIFY GROUP_REF FOR ALL ACTIONS NOT JUST NEW, OPEN, MOUNT?

                                    # --- IF ACTION REQUEST AND THERE WERE NO PROCESSING ERRORS OF THE GROUP_REF, CONTINUE
                                    if act in ['new', 'open', 'mount'] and d6_rc == '0':
                                        # --- CALL CHECK_MODULE, TO GET THE GROUP ASSIGNMENT, IF ANY
                                        log.debug("command - calling get_group for modules in slots %s " %( lparam))
                                        gr_MSN = module_libs.get_group(lparam)
                                        #print "return get_group with ", gr_MSN
                                        if act == 'new' and len(gr_MSN) > 0: 
                                            msg = "command - new group, but module belongs to group %s " % (gr_MSN)
                                            State.add_error_msg(msg)
                                            gref = ''.join(lparam)
                                            d6_rc = '2'

                                        elif act in ['open', 'mount']:
                                            if not Utils.verify_group(lparam):
                                                mod_ver = module_libs.verify_modules(gr_MSN)
                                                if not mod_ver['rv']:
                                                    msg = "command - open group, but all modules in group %s are not present" % (mod_ver['missing'])
                                                    State.add_err_message(msg)
                                                    gref = ''.join(lparam)
                                                    d6_rc = '2'
                                                    cp_rc = '31'   # --- TRYING TO OPEN AN INCOMPLETE GROUP
                                                

                                        #log.debug("command - slot count (%d), number of slots in group (%d)" %(slot_cnt, len(lgroup)))
                                        # --- IF A GROUP IS OPEN AND REQUEST IS OPEN SEND ERROR 
                                        #if slot_cnt == len(lgroup) and d6_rc != '2':
                                        log.debug("command - slot count (%d), number of slots in group (%d)" %(slot_cnt, len(param)))
                                        # --- IF A GROUP IS OPEN AND REQUEST IS OPEN SEND ERROR 
                                        if slot_cnt == len(lparam) and d6_rc != '2':
                                            # (CHET) think about this for a new disk and raid, group, eMSN and serial numbers cannot be 
                                            # generated till mounted.

                                            log.debug("command - group modules %s " %( lparam))
                                            rv  = State.group(act, lparam)
                                            log.debug("Command - returned from group with %s" % rv)
                                            gref = str(rv['gref'])

                                            # --- UPDATE THE SLOTS_STATUS GROUP
                                            d6_rc = rv['rv']
                                            #print "d6_rc ", d6_rc

                                            if d6_rc == '0' and act == 'new':
                                                for si in lparam:
                                                    State.slot_status[int(si)]['group'] = gref
                                                    log.info("Command - updated group for slot %s to %s" % (si, gref))

                                            free_space = module_libs.getremainingspace(gref)
                                            log.debug("Command - free space remainig on disk is %d" % free_space)
                                            
                                            # --- IF THE MODULE WAS SUCCESSFULLY GROUPED AND MOUNTED CHECK THE META-DATA IS PROPERLY STORED
                                            #if act == "new" and d6_rc == '0':
                                            #    log.debug("Command : updating group information %s" % gref)
                                            #    ret = modules_lib.create_group(lparam)
                                        
                                            # --- IF A SUCESSFUL OPEN OCCURRED, UPDATE THE STATUS
                                            if act == "open" and d6_rc == '0':
                                                log.debug("Command : updating status for group_ref %s" % gref)
                                                State.c_status |= module_libs.modules_status(gref)
                                                log.info("Command : update status to 0x%07x " % State.c_status)
                                                
                                                # --- UPDATE THE NUMBER OF DEVICES FOR RECORDING
                                                State.num_devs = Utils.open_storage_dev(gref)
                                                log.info("Command : updated number of devices for recording to %d" % State.num_devs)
                                                # --- UPDATE THE DEVICE LIST FOR DPLANE OF DEVICES FOR RECORDING
                                                State.devs_list = Utils.open_record_dev(gref)
                                                log.info("Command : updated device list used  %s" % State.devs_list)

                                            elif act == "mount" and d6_rc == '0':
                                                log.debug("Command : updating status for group_ref %s" % gref)
                                                State.c_status |= module_libs.modules_status(gref)
                                                log.info("Command : update status to 0x%07x " % State.c_status)

                                            
                                        else:
                                            msg = "Not all slots to be grouped are available, or group_ref was not complete check mstat? output"
                                            State.add_error_msg(msg)
                                            d6_rc = '8'
                                            cp_rc = '31' # --- Not all slots to be grouped are available, or group_ref was not complete check mstat?
                                    
                                    elif act in ['protect', 'unprotect'] and d6_rc == '0':
                                        log.debug("command - set protection on group %s to %s" %( param, act))
                                        group_ref = lparam
                                        rv  = State.group(act, group_ref )
                                        gref = str(rv['gref'])
                                        # --- UPDATE THE SLOTS_STATUS GROUP
                                        d6_rc = rv['rv']

                                    elif act == 'erase' and d6_rc == '0':
                                        group_ref = lparam
                                        gref = ''.join(lparam)
                                        if State.unprotect_flag == 1:
                                            log.info("info - Erase modules in slots %s " %( lparam))
                                            slot_type = State.slot_status[int(lparam[0])]['type']
                                            rv  = module_libs.delete_all_scans(gref,  slot_type)
                                            #gref = str(rv['gref'])
                                            log.info("Now ERASING %s" % gref)
                                            #  d6_rc = rv['rv']
                                            d6_rc = str(rv)
                                        else:
                                            log.warning("group=unprotect must be issued immediately before group=erase, failed to erase")
                                            d6_rc = '4'
                                            cp_rc = '32'  # --- Unprotect required immediately before erase

                                    elif d6_rc == '0':
                                        log.debug("Command -  unmount group %s" % State.open_group)
                                        
                                        # --- GET THE GROUP_REF
                                        group_ref = lparam
                                        rv  = State.group(act, group_ref )
                                        gref = str(rv['gref'])
                                        d6_rc = rv['rv']
                                        # --- IF MODULES SUCCESSFULLY UNMOUNTED, CALL TO UPDATE MODULES STATUS:
                                        #if d6_rc == '0':                                            
                                        #    (rv, rl) = module_libs.modules(lparam)

                                    else:
                                        # --- IF THE PARAMETER IS NOT ALL NUMBERS, THAN ERROR:
                                        log.warning("group command the rc was not valid for processing for  %s " % param)
                                        #State.add_error_msg(msg)
                                        # --- DO NOT HAVE TO ASSIGN RETURN VALUE, ILLEGAL REQUEST
                                        gref = param
 
                                else:
                                    # --- IF THE PARAMETER IS NOT ALL NUMBERS, THAN ERROR:
                                    msg =  "2 - group_ref as not a valid entry %s " % param
                                    State.add_error_msg(msg)
                                    d6_rc = '8'
                                    gref = param


                        except AttributeError:
                            msg = "Command : GroupCommand - No PARAM FIELD PROVIDED" 
                            State.add_error_msg(msg)
                            d6_rc = '8'
                    else:
                        # --- GET UNASSIGNED SLOTS THAT HAVE POWER AND NOT ASSIGNED A GROUP
                        param = ''
                        for i in State.slot_status.keys():
                            if State.slot_status[i]['group'] == '-' and  State.slot_status[i]['status1'] == 'initialized' and State.slot_status[i]['type'] == 'sg':
                                param = param + str(i)
                        
                        if param != '':
                            log.debug("Command - auto grouping requested using module(s) %s" % param)
                            rv = State.group(act, param)
                            gref = str(rv['gref'])
                            d6_rc = rv['rv']
                            if not d6_rc : 
                                # --- SINCE IT IS AUTO, NO PARAM WERE DEFINED, A SUCCESS RESULTED IN PARAM
                                lparam = list(param)
                                for si in lparam:
                                    State.slot_status[int(si)]['group'] = gref
                                    log.info("Command - updated group for slot %s to %s" % (si, gref))

                        else:
                            msg = "auto grouping requested failed, no SG modules exist " 
                            State.add_error_message(msg)
                            d6_rc = '8'
                            cp_rc = '33' # --- Auto grouping failed no sg modules 

                elif act == 'close':
                    log.debug("Command - close open group %s" % State.open_group)
                    
                    # --- (CHET) NEED TO CHECK IF THE GROUP IS ALEADY CLOSED, IF NOT CONTINUE
                    # --- GET THE OPEN GROUP_REF
                    group_ref = State.open_group['slot']
                    rv  = State.group(act, group_ref )
                    gref = str(rv['gref'])
                    d6_rc = rv['rv']
                    if d6_rc == '0':
                        State.open_group['slot'] = '-'
                        State.open_group['type'] = '-'                        
                        log.debug("Command : updating status for group_ref %s" % gref)
                        # --- CLEAR  THE BANKS STATUS
                        State.c_status &= 0xfff
                         # --- UPDATE MODULE STATUS AND OR IT WITH STATUS
                        State.c_status |= module_libs.modules_status(gref)
                        log.info("Command : update status to 0x%07x " % State.c_status)

                        # --- RESET THE NUMBER OF DEVICES FOR RECORDING
                        State.num_devs = 0 
                        log.info("Command : updated number of devices for recording to %d" % State.num_devs)
                        
                        # --- UPDATE THE DEVICE LIST FOR DPLANE OF DEVICES FOR RECORDING
                        State.devs_list = [] 
                        log.info("Command : cleared the device list for recording to %s" % State.devs_list)
                        
                        State.fmask = 0
                        log.info("Command : cleared the fmask for recording to %s" % State.fmask)

                else:
                    msg = "Command : GroupCommand - %s IS INVALID" % act
                    State.add_error_msg(msg)
                    d6_rc = '8'

        except AttributeError:
            msg = "Command : GroupCommand - No ACTION FIELD PROVIDED" 
            State.add_error_msg(msg)
            d6_rc = '8'

        if act != 'unprotect' and State.unprotect_flag == 1:
            State.unprotect_flag = 0
            log.info("resetting unprotect_flag")

        r = Response.SetGroupCmd(return_code=d6_rc,
                                 cplane_return_code = cp_rc,
                                 group_ref = gref,
                                 list=l)
        
        log.debug("Command: GroupCommand Exit with %s" %r)

        return str(r)    

 ###################### group? ########################################
class GroupQuery(Query):
    PARAMS = [
        ]
    
    def execute(self):
        log.debug('Command:GroupQuery Entered:')
        l = []
        
        if len(State.open_group) == 0:
            # --- IF NO GROUPS ARE DEFINED, GET THE INFORMATION
            lslots = len(State.unassigned_slots) 
            if lslots == 4:
                log.info("Command GroupQ() No slots have been assigned")
                gref = 'NA'

            else:
                num_slots = 4 - lslots
                log.info("Command %d slots have been assigned" % (num_slots))
                gref = State.open_group['slot']
                         
         #--- GET THE NUMBER OF UNASSIGNED MODULES AND RETURN AVAILABLE DISKS
        else:
            log.debug("Command, open group is %s " % State.open_group)
            gref = State.open_group['slot']
            
        r = Response.GetGroupCmd(return_code='0',
                                 cplane_return_code='0',
                                 group_ref=gref,
                                 list=l)

        log.debug("Command: GroupQuery Exit with %s" %r)

        return str(r)    
 ###################### group_members? ########################################
class GroupMembersQuery(Query):
    PARAMS = [
        'slot'
        ]
    
    def execute(self):
        log.debug('Command:GroupQuery Entered:')
        l = []
        

        d6_rc = '0'
        slot_q = '-'

        # --- IF A PARAMETER WAS NOT PASSED, AN ATTRIBUTE ERROR IS RAISED AND AN ERROR
        try:
            if self.slot != None:
                slot_q = self.slot
                log.debug("Command : group_members? for slot %s" % (slot_q))

                islotq = int(slot_q)
                if islotq in [1,2,3,4]:
                    # --- GET THE ASSOCIATED GROUP-MEMBERS EMSN

                    tmp = [slot_q]
                    # --- CHECK TO SEE IF INFORMATION EXIST ABOUT THE SLOT
                    if State.slot_info[islotq].keys() :
                        # --- GET THE GROUP INFORMATION RELATED TO THE MODULE
                        gg_rv = module_libs.get_group(tmp)
                        # --- IF THE LENGTH OF GET_GROUP > 0 THEN INFORMATION EXISTS IN GROUP
                        if len(gg_rv) > 0:
                            gg_rv_l = gg_rv[0].split(":")
                            # --- remove the number of eMSN's
                            gg_rv_l.pop(0)
                            
                            for i in gg_rv_l:
                                # --- THE RESPONSE EXPECTS A LIST FOR EACH ENTRY
                                ap = [i]
                                l.append(ap)

                        # --- ELSE IF A RAID, SINCE ONLY ONE MODULE, USE SLOT_STATUS INFO
                        elif State.slot_status[islotq]['type'] == 'raid':
                            ap = [State.slot_status[islotq]['eMSN']]
                            l.append(ap)
                        
                        d6_rc = '0'
                    else:
                        d6_rc = '4'
                        msg = "Command : GroupMember? - no information on the slot %s " % slot_q 
                        State.add_error_msg(msg)

                #--- GET THE NUMBER OF UNASSIGNED MODULES AND RETURN AVAILABLE DISKS
                else:
                    msg = ("Command : group_members? has invalid slot # %s " % slot_q)
                    State.add_error_msg(msg)
                    d6_rc = '8'

        except AttributeError:
            msg = "Command : group_members? - No SLOT PROVIDED" 
            State.add_error_msg(msg)
            d6_rc = '8'

        r = Response.GetGroupMembers(return_code=d6_rc,
                                        cplane_return_code='0',
                                        slot=slot_q,
                                        list=l)

        log.debug("Command: GroupQuery Exit with %s" %r)

        return str(r)    
    
################ gsm ########################################
class GsmQuery(Command):
    PARAMS = [
        'volref',
        ]

    def execute(self):
        log.debug('Command:GsmQuery Entered:')
        l = []

        rv = State.get_gsm_mask()
        #print "returned from class CState - Persistent with rv = %d", rv

        r = Response.GetGSM(return_code='0',
                            cplane_return_code='2',
                            list=l)

        log.debug("Command: GsmQuery Exit with %s" %r)

        return str(r)    
    
class GsmCommand(Command):
    PARAMS = [
        'volref',
        'GSM',
        ]

    def execute(self):
        log.debug('Command:GsmCommand Entered:')
        l = []

        r = Response.SetGSM(return_code='0',
                            cplane_return_code='2',
                            list=l)

        log.debug("Command: GsmCommand Exit with %s" %r)

        return str(r)    

####################### gsm_mask ####################################    
class GsmMaskCommand(Command):
    PARAMS = [
        'erase_mask_enable',
        'play_mask_enable',
        'record_mask_enable',
        ]
    def execute(self):
        log.debug('Command:GsmMaskCommand Entered:')
        log.debug('Command:GsmMaskCommand eme is -%d-, pme is -%d-, rme is -%d- ' % (self.erase_mask_enable,self.play_mask_enable,self.record_mask_enable))
        l = []

        r = Response.SetGsmMask(return_code='0',
                            cplane_return_code='2',
                            list=l)

        log.debug("Command: GsmCommand Exit with %s" %r)

        return str(r)    

class GsmMaskQuery(Command):
    PARAMS = []

    def execute(self):
        log.debug('Command:GsmMaskQuery Entered:')
        l = [State.gsm_mask_vals[0],    # erase 
             State.gsm_mask_vals[1],    # play
             State.gsm_mask_vals[2]]    # record

        r = Response.GetGsmMask(return_code='0',
                            cplane_return_code='2',
                            list=l)

        log.debug("Command: GsmCommand Exit with %s" %r)

        return str(r)    

###################### undocumented dterm ##############################
''' this command terminates d-plane program '''
class DtermCommand(Command):
    PARAMS = []

    def execute(self):
        log.debug('Command:dtermCommand Entered:')
        l = []
        s = State.dterm()
        r = Response.SetDterm(return_code='0',
                            cplane_return_code='2',
                            list=l)

        log.debug("Command: dtermCommand Exit with %s" %r)

        return str(r)    
###################### undocumented cterm ##############################
''' this command terminates c-plane program '''
class CtermCommand(Command):
    PARAMS = []

    def execute(self):
        log.debug('Command:ctermCommand Entered:')
        l = []
        r = Response.SetCterm(return_code='0',
                            cplane_return_code='2',
                            list=l)

        log.debug("Command: CtermCommand Exit with %s" %r)
        #exit(0)
    
        return str(r)    

############### delete ################################
class DeleteCommand(Command):
    PARAMS = ['scans']

    def execute(self):
        log.debug('Command:DeleteCommand Entered:')
        l = []
        s = 4   # return code is error encounterd during attempt to execute
        ogrp = State.open_group

        # --- IS THERE AN OPEN GROUP
        try:
            if self.scans != None: 
                sn = self.scans
                if ogrp['slot']!= '-':
                    log.debug("delete : There is an open group %s" % ogrp['slot'])

                    # --- IF WE ARE NOT RECORDING
                    if State.record_session['action'] != 'on':
                        s = module_libs.delete_scan(ogrp['slot'], sn, ogrp['type'])
                        if s == 0:
                            State.last_scan_removed = sn
                    else:
                        msg = 'Cannot delete a scan while recording, try again later'
                        s = 5
                else:
                    msg = "There is no open group, cannot delete %s " % sn
                    State.add_error_msg(msg)
                    s = 8
        except AttributeError:
            msg = "Command : delete - No scan name provided" 
            State.add_error_msg(msg)
            s = 8

        r = Response.SetDelete(return_code=str(s),
                            cplane_return_code=str(s),
                            list=l)

        log.debug("Command: DeleteCommand Exit with %s" %r)

        return str(r)    

class DeleteQuery(Command):
    PARAMS = [
        'volref',
        ]

    def execute(self):
        log.debug('Command:DeleteQuery Entered:')
        l = []
        sn = State.last_scan_removed
        if sn == '':
            sn = '-'

        r = Response.GetDelete(return_code='0',
                               cplane_return_code='0',
                               scan_name=sn,
                               list=l)

        log.debug("Command: DeleteQuery Exit with %s" %r)

        return str(r)    

###################### execute= #########################
class ExecuteCommand(Command):
    PARAMS = [
        'action',
        'filename',
        'data',
        ]

    def execute(self):
        log.debug('Command:ExecuteCommand Entered:')
        l = []
        dc_rc = '8' 
        d6_rc = '0'      # --- SET THE CPLANE RETURN CODE TO SUCESS, 
        fn = "NULL"

        try:
            if self.action != None:
                act = self.action
                if act in ['upload', 'append', 'finish', 'delete', 'restart', 'reboot']:
                    log.debug("Command:ExecuteCommand received action = %s" % act)
                    # --- THE FILE NAME HAS TO BE PROVIDED
                    try:
                        if self.filename != None:
                            fn = self.filename
                            log.info("Command:ExecuteCommand begin %s for file %s " % (act,fn))
                            # --- SET THE STATUS ENABLED
                            dc_rc = '1'

                    except AttributeError:
                        msg = "Second field of execute command is required, none provided"
                        State.add_error_msg(msg)
                        dc_rc = '3'
                        d6_rc = '21' # NO FILENAME FIELD VALUE PROVIDED, EXPECTED

                else:
                    msg = "Command:ExecuteCommand INVALID ACTION PROVIDED - %s" % act
                    State.add_error_msg(msg)
                    d6_rc = '21' # NO VALID ACTION FIELD VALUE PROVIDED, EXPECTED
                        

        except AttributeError:
            msg = "Command:ExecuteCommand NO ACTION PROVIDED"
            State.add_error_msg(msg)
            d6_rc = '21' # NO VALID ACTION FIELD VALUE PROVIDED, EXPECTED

        if dc_rc == '1':
            if act in ['append', 'finish'] and fn == State.fn:
                log.debug("Command:ExecuteCommand : appending data ")
                if fn == State.fn:
                    # --- APPEND THE CONNTENTS TO FILE 
                    ofn = open(State.ffn, 'a')
                    log.debug("CommandExecute - Appending data to %s" % fn)
                    # --- UPDATE THE LAST ACTION 
                    State.ex_last_action = act

                else:
                    msg = ("CommandExecute - filename missmatch occured.  Received %s, expected %s" % (fn, State.fn))
                    State.add_error_msg(msg)
                    dc_rc = '8'
                    d6_rc = '22' # FILENAME NOTE CONSISTENT FOR APPEND OR FINISH 
                    
            elif act == 'upload' and State.ex_last_action == 'finished':
                log.debug("Command:ExecuteCommand : creating new file and begin upload process ")
                 # --- FILE LOCATION IS DEFAULTED TO /HOME/OPER/ UNLESS A FULL PATH IS GIVEN
                State.fn = fn
                if State.fn.find("/") != 0:
                    #State.ffn = "/home/oper/" + State.fn
                    # --- GET THE USER RUNNING THE PROGRAM
                    uname = getuser()
                    tmp_ffn = "/home/"+uname+"/.mark6"
                    if not os.path.isdir(tmp_ffn):
                        os.mkdir(tmp_ffn)

                    State.ffn = tmp_ffn+"/"+State.fn

                else:
                    State.ffn = State.fn

                # --- DOES THE FILE EXIST?, IF SO ERROR
                if os.path.isfile(State.ffn):
                    msg = "FILE TO START UPLOADING ALREADY EXISTS (-%s-) USE DIFFERENT FILE NAME" % State.fn
                    State.add_error_msg(msg)
                    dc_rc='4'
                    d6_rc = '23' # NO VALID ACTION FIELD VALUE PROVIDED, EXPECTED

                # --- ELSE CREATE THE FILE
                else:
                    ofn = open(State.ffn, 'w')
                    # --- UPDATE THE LAST ACTION 
                    State.ex_last_action = act

            elif act =='delete':
                # --- GET THE USER RUNNING THE PROGRAM
                uname = getuser()
                # --- assume it is un users .mark6 directory
                ffn = "/home/"+uname+"/.mark6/" + fn

                log.info("attempting to remove uploaded file %s" % ffn)

                if os.path.isfile(ffn):
                    os.remove(ffn)
                    log.info("CommandExecute - execute=deleted file %s" % fn)
                    dc_rc='0'
                    

                    # --- CLEAR THE STATE VARIABLE
                    if State.ffn == ffn:
                        State.ffn = 'NULL'
                        log.debug("CommandExecute - State data updated from %s to NULL" % ffn)

                    if State.fn == fn:
                        State.fn = 'NULL'
                        log.debug("CommandExecute - State data updated from %s to NULL" % fn)
                else:
                    msg = "CommandExecute - execute=delete file %s DOES NOT EXIST" % (ffn)
                    State.add_error_msg(msg)
                    dc_rc='4'
                    d6_rc = '25' # TRIED TO REMOVE A FILE THAT DOES NOT EXIST 

            # --- RESTART cplane and dplane APPLICATION
            elif act =='restart' or act == 'reboot':
                if State.record_session['action'] == 'off' :
                    log.info("CommandExecute - execute=restart/reboot: Mark6 application service - NYI")
                    dc_rc='2'
                    if self.data == 'last':
                        # --- SAVE PRESENT STATE
                        log.debug("saving State information")
                        State.save_state()
                    
                    elif self.data == 'clean':           
                        # --- REMOVE STATE CONFIGURATION INFO
                        rv =State.config_remove()
                        if rv == 1:
                            d6_rc = '26' # CONFIG.XML FILE WAS NOT SAVED, REPORT AS BUG TO DEVELOPER
                            dc_rc = '4'
                    else:
                        log.warning("CommandExecute = invalid variable %s supplied, must be {clean, last}")
                        dc_rc = '8'

                    if dc_rc != '8':
                        if act == 'restart':
                            log.waring("Setting the Restart flag for the Mark6 applications")
                            State.restart = 1
                        else:
                            log.warning("Rebooting the Mark6")
                            # --- unmount the disk modules
                            # --- then set restart = 6 for shutdown of system after response is made.

                else:
                    msg = "Trying to restart / reboot the system when record state is not OFF"
                    State.add_error_msg(msg)
                    dc_rc = '4'
                
            else:
                log.warning("Command:ExecuteCommand : invalid squence occured, filename %s, expected %s, last action %s" % fn, State.fn, State,ex_last_action)
                dc_rc = '8'
                d6_rc = '23' # INVALID UPLOAD SEQUENCE ATTEMPTED 
                
            if dc_rc == '1':
                # --- WRITE THE CONTENTS TO THE FILE
                ofn.write(self.data)
                # --- CLOSE THE FILE
                ofn.close()


            # --- IF THE ACTION WAS TO FINISH THE FILE, CHANGE THE STATE.FN
            if act == 'finish':
                log.info("FILE %s HAS BEEN FINISHED WRITING AND CLOSED" % (State.fn))
                State.fn = 'NULL'
                State.ex_last_action = 'finished'
                dc_rc = '0'
        else:
            log.warning("Command:execute did not have proper syntax, or the filename missmatch occured")
            # --- CHECK AND ADD CPLANE MORE SPECIFIC RETURN CODE HERE.
            
            
        r = Response.SetExecute(return_code=dc_rc,
                            cplane_return_code = d6_rc,
                            list=l)

        log.debug("Command: ExecuteCommand Exit with %s" %r)

        return str(r)    
############################## execute? ################################################
class ExecuteQuery(Command):
    PARAMS = [
        'action',
        'type',
        ]

    def execute(self):
        log.debug('Command:ExecuteQuery Entered:')
        l = []
        action = "-"
        fn="-"
        out=""

        # --- IS THERE AN ACTION?
        try:
            if self.action != None:
                action = self.action
                if action != "sys_info":
                    msg = "illegal action was provided -%s-"  % action
                    State.add_error_msg(msg)
                    dc_rc='3'
                else:
                    dc_rc='1'

        except AttributeError:
            log.info("Command:execute? was made and will check the state of upload")

        if action == '-':
            action = State.ex_last_action
            fn = State.fn
            dc_rc = '0'
        elif action == "sys_info":
            cmd = ["m6sensors.sh", "zip"]
            (rv, out) = Utils.linux_cmd(cmd)
            if rv == 0:
                msg = out
                State.add_error_msg(msg)
                dc_rc='4'
            else:
                dc_rc='0'

        else:
            dc_rc='8'
            

        r = Response.GetExecute(return_code=dc_rc,
                               cplane_return_code='0',
                               action=action,
                               file_name=fn,
                               output=out,
                               list=l)

        log.debug("Command: ExecuteQuery Exit with %s" %r)

        return str(r)    

################### list= ############################
class ListCommand(Command):
    PARAMS = []

    def execute(self):
        log.debug('Command:ListCommand Entered:')
        l = []
        # --- NOT IMPLEMENTED YET
        
        #s = State.delete_scan()
        r = Response.SetList(return_code='2',
                            cplane_return_code='2',
                            list=l)

        log.debug("Command: ListCommand Exit with %s" %r)

        return str(r)    

################### list? ############################
class ListQuery(Command):
    PARAMS = [
        'group_ref'
        ]

    def execute(self):
        log.debug('Command : ListQuery Entered:')
        l = []
        sn = "-"
        length = "-"
        datestamp ="-"
        gref = "-"
        rc = '2'  
        num = " " + str(0) + " " 
        sn = " - "
        size = " - "
        ctime = " - "
        gref = '-'
        not_good_gref = 1
        try:
            if self.group_ref != None:
                gref = self.group_ref
                #print "gref set to ", gref
                log.debug("Command : ListQuery - request for group_ref %s" % (gref))
                log.debug("Command : ListQuery - Verifying  %s" % (gref))
                lgref = list(gref)
                # --- IF GROUP_REF IS THE OPEN GROUP
                if State.open_group['slot'] == gref:
                    file_list = State.file_dict
                    not_good_gref = 0
                # --- IF NOT A OPEN GROUP, IS THE GROUP IS VALID AND READABLE
                elif Utils.verify_group(lgref):
                    # --- IF the group is open / closed for status1
                    if Utils.readable(lgref):
                        # --- IF NOT RECORDING READ THE DIRECTORY
                        file_list = module_libs.read_list(gref)
                        #print "read file_list ", file_list
                        not_good_gref = 0
                    else:
                        log.warning("The group ref %s is not readable" % gref)
                        # --- SET THE GREF TO '-' SO IT IS NOT PROCESSED.

                else:
                    log.debug("The group ref passed is not valid, or an open group")

        except AttributeError:
            if State.open_group['slot'] != "-":
                log.info("Command : ListQuery - open gref will be used %s" % gref )
                gref = State.open_group['slot']
                file_list = State.file_dict        
                not_good_gref = 0
            else:
                log.info("Command : ListQuery - no gref available")
                                
        if not not_good_gref:
            # --- this needs to be in a list
            log.debug("list : getting directory list")
            num_scans = len(file_list.keys())
            log.debug("list : num of scans that exist %d" % (num_scans))
            rc = '0'
            if num_scans != 0:
                ord_list_sn = sorted(file_list.keys())
                
                for i in ord_list_sn:
                    num = " " + str(i) + " " 
                    sn = " "+file_list[i]['sn'] + " "
                    size = " "+str(file_list[i]['size'])+ " "
                    ctime = " "+file_list[i]['create_time'] + " "
                    l.append([num, sn, size, ctime])
            else:
                l.append([num, sn, size, ctime])
        else:
            l.append([num, sn, size, ctime])
                
        r = Response.GetList(return_code=rc,
                             cplane_return_code='0',
                             group_ref=gref,
                             list=l)

        log.debug("Command: ListQuery Exit with %s" %r)

        return str(r)    

################### m6cc= ############################
class M6ccCommand(Command):
    PARAMS = [
        'action',
        'config_file',
        ]

    ''' TTD: need to add the abort case '''

    def execute(self):
        log.debug('Command:M6ccCommand Entered:')
        l = []
        # --- will launch a command application with the appropriate configure file
        log.debug('Query:MstatQuery Entered:')
        l = []
        # --- SET THE RETURN CODE TO SUCCESS
        rc = '0'
        cp_rc = '0'             # --- cplane unique return code
        cf = "NULL"             #config file init
        act = "NULL"            # action init

        try:
            if self.action != None:
                act = self.action
                if act == 'execute':
                    log.debug("Command:M6ccCommand valid action %s" % act)
                    try:
                        if self.config_file != None:
                            cf = self.config_file
                            # --- IF THE CONFIG FILE DOES NOT EXIST, SYNTAX ERROR
                            uname = getuser()
                            State.ffn = "/home/"+uname+"/.mark6/" + cf

                            log.debug("ffn is %s" % (State.ffn))

                            # --- DOES THE FILE EXIST?, IF SO ERROR
                            if os.path.isfile(State.ffn):
                                # --- PROCESS THE ACTION
                                log.debug("ffn does exist, perform action %s" % (act))
                                rc = '1'
                                State.m6cc_state = 'active'

                            else:
                                msg="The file for M6cc to operate on (%s) does not exist, please upload" % State.ffn
                                State.add_error_msg(msg)
                                rc = '3'                                
                                #  --- SET THE M6CC_STATE TO ERROR
                                State.m6cc_state = 'error'
                                # --- SET THE CPLANE RETURN CODE FOR CONFIGURATION FILE NOT FOUND
                                cp_rc = '80'

                    except AttributeError:
                        log.warning("Command: M6cc Command config file was not provided" % act)
                        rc = '3'

                elif act == 'abort':
                    log.info("Attempting to abort the M6cc process")

        except AttributeError:
            log.warning("Command:M6ccCommand no action %s" % act)
            rc = '3'


        if act == 'execute' and rc == '1':
            log.info("M6CC will %s on %s " % (act, State.ffn))
            if act == 'execute':
                # --- IS A GROUP FOR M6CC
                if State.open_group['slot'] == '-':
                    msg = "A group is not open for M6CC, please open a group before issueing cmd"
                    State.add_error_msg(msg)
                    rc = '4'
                    cp_rc = '2'   # --- general return code, no open group available

                # --- ELSE IS A M6_CC ACTIVE?
                else:
                    if Utils.get_m6cc() == '0':
                        State.m6cc_state = 'active'
                        cmd = ['M6_CC',  '-f', State.ffn]
                        State.m6cc_pid = subprocess.Popen(cmd).pid
                        log.info("m6cc_pid %d" % ( State.m6cc_pid))
                        rc='0'
                    else:
                        msg = "M6_CC is executing, please abort the process, cannot proceed"
                        State.add_error_msg(msg)
                        rc = '4'
                        cp_rc = '81'   # --- PID for m6cc found, cannot continue

            # --- ELSE IT IS AN ABORT
            else:
                if State.m6cc_state == 'active':
                    pid = Utils.get_m6cc()
                    if pid != 0:
                        log.info("m6cc is active, attempting to abort")
                        cmd = ['kill', '-9', str(State.m6cc_pid)]
                        (rt, out) = Utils.linux_cmd(cmd)
                        # --- IF SUCCESSFUL
                        if rt == 1:
                            # MAKE VSI COMPLIANT
                            rc = '0'
                            # CHANGE STATE TO ABORT and RESET THE PID
                            State.m6cc_state = 'abort'
                            State.m6cc_pid = 0
                        else:
                            rc = '4'
                            cp_rc = '82'   # --- abort failed
                            msg = "Abort of M6_CC failed"
                            State.add_error_msg(msg)
                    else:
                        log.warning("M6_CC attempted to abort a process that is not active")
                        rc= '4'
                        cp_rc = '83'    # --- Aborting a non-active m6cc
                        
        r = Response.SetM6cc(return_code=rc,
                            cplane_return_code='0',
                            list=l)

        log.debug("Command: M6ccCommand Exit with %s" %r)

        return str(r)    
##################### m6cc? ##############################################
###################### M6cc? ##########################################3
class M6ccQuery(Command):
    PARAMS = []

    def execute(self):
        log.debug('Query:M6ccQuery Entered:')
        l = []
        dc_r6 = '0'

        if State.m6cc_state == 'active':
            dc_r6 = Utils.get_m6cc()

        # --- this needs to be in a list
        r = Response.GetM6cc(return_code=dc_r6,
                             cplane_return_code='0',
                             config_file=State.ffn,
                             state=State.m6cc_state,
                             list=l)

        log.debug("Command: ExecuteQuery Exit with %s" %r)

        return str(r)    

################ mstat= ###############################
class MstatCommand(Command):
    PARAMS = [
        ]

    def execute(self):
        log.debug('Command: MstatCommand Entered:')
        l = []

        # --- COMMAND DOES NOT EXIST, RETURN 2
        r = Response.SetMstat(return_code='2',
                            cplane_return_code='2',
                            list=l)

        log.debug("Command: MstatCommand Exit with %s" %r)

        return str(r)    

######################## mstat? ###############################
class MstatQuery(Command):
    PARAMS = [
        'type'
        ]

    def execute(self):
        log.debug('Query:MstatQuery Entered:')
        l = []
        # --- SET THE INTERGER GROUP TO ALL
        ity = 0
        # --- SET THE RETURN CODE TO SUCCESS
        rc = 0
        d6_rc = 0
        slot = []

        try:
            if self.type != None:
                ty = self.type
                # --- TEST TO SEE IF IT IS ONLY NUMBERS
                if ty.isdigit():
                    # --- either groupref or slot
                    # --- INT OF TYPE
                    ity = int(ty)
                    if ity > 0 and ity < 5:
                        log.info("mstat information on slot %d requested" % (ity))
                        #print "slots available", State.slot_info.keys()
                        if ity in State.slot_info.keys():
                            log.debug("Command: mstat requesting for valid slot %d" % (ity))
                            log.debug("slot %d  is %s" % (ity, State.slot_info[ity]))
                            #for i in range(8):
                            #    print "i is ", i
                            #    print "slot %d disk %d is %s" % (ity, i, State.slot_info[ity][i])

                        # --- assign ty to slot
                        ty = 'slot'
                        #print "slot %d disk %d" % (ity, i)                    
                    else:
                        log.info("mstat information on group %d requested" % (ity))
                        # --- DETEREMINE IF IT IS VALID GROUP, ORDER DOES NOT MATTER 
                        lty = len(ty)
                        if lty <= 4:
                            log.debug("mstat: length of group is good")
                            if lty == 1:
                                # --- CHECKED IF ONLY 1 DIGIT IT IS VALID ABOVE THEREFORE ERROR
                                msg = "mstat: group is not between in 1-4 slots"
                                State.add_error_msg(msg)
                                rc = 3
                            else:
                                # --- CONVERT THE SLOT LIST TO INTEGERS 
                                slot_list = map(int, list(ty))
                                # --- SET THE TY TO SLOTS FOR COMPARISON LATER, SORT IN ASCENDING ORDER
                                slot_list.sort()
                                # --- CREATE THE GROUP STRING
                                sslots = ''.join(map(str, slot_list))

                                for i in slot_list:

                                    if i > 0 and i <= 4:
                                        log.debug("Command :  valid slot")
                                        ty = 'group'
                                    else:
                                        msg = "mstat: invalid slot"
                                        State.add_error_msg(msg)
                                        rc = 3
                        else:
                            msg = "mstat: group is > 4 digits"
                            State.add_error_msg(msg)
                            rc = 3

                        ty = 'group'
                        log.debug("Command : setting type to group")

                else:
                    log.debug("Command : information is not slot or group")                        

        except AttributeError:
            ty = 'open'
            log.debug("Command: setting type to default - open") 

        # --- CHANGE TO LOWER CASE
        ty = ty.lower()
            
        # --- IF OPEN, ALL OR NULL 
        if ty in ['open', 'all', 'null', 'slot', 'group'] and rc == 0:
            sl = []
            # --- IF TYPE = OPEN, THEN RETURN ALL MODULES WITH STATUS1 OPEN
            if ty == 'open':
                for i in range(1,5):
                    if State.slot_status[i]['status1'] == 'open':
                        sl.append(i)
                
                log.debug("Command:mstat - Calling for open modules(%s)" % (sl))
                (rc, l) = module_libs.modules(sl)
                log.debug("Command:mstat - returned modules(open) = %s" % (l))

            # --- ELSE IF TYPE ALL / NULL, RETURN MOUNTED MODULES INFORMATION
            elif ty in ['all', 'null']:
                sl = [1,2,3,4]
                log.debug("Command:mstat - Calling for open modules(%s)" % (sl))
                (rc,l) = module_libs.modules(sl)
                log.debug("Command:mstat - returned modules(open) = %s" % (l))                

            # --- ELSE IF SLOT, RETURN ONLY THAT SLOT INFORMATION
            elif ty == 'slot':
                log.debug("Command:mstat - calling modules(%d)" % (ity))            
                sl.append(ity)
                (rc, l) = module_libs.modules(sl)
                log.debug("Command:mstat - returned modules(%d) %s" % (ity, l))                

                #print "slots available", State.slot_info.keys()
                #for i in State.slot_status[ity]:
                #    print "slot %d disk %d is %s" % (ity, i, State.slot_info[ity][i])

            # --- ELSE IT MUST BE A GROUP, SO GET THE GROUP INFORAMTION
            else:

                log.debug("Command:mstat information on slot %s requested" % (ty))

                # --- SLOT_VALID FLAG = VALID
                slot_valid_fl = 1
                # --- VERIFY IT IS A VALID GROUP, IF YES GET THE INFORMATION.  IF NOT RETURN INVALID REQUEST
                for i in slot_list:
                    log.debug("checking group slot %d, %s against %s" % (i, State.slot_status[i]['group'], ty))
                    if State.slot_status[i]['group'] != sslots:
                        slot_valid_fl &= 0
                        
                if slot_valid_fl:
                    (rc, l) = module_libs.modules(slot_list)
                    log.debug("Command:mstat - returned modules(%d) %s" % (ity, l))                
                else:
                    # --- cplane error code for invalid group_ref
                    d6_rc = 110
                    # --- INVALID PARAMTER
                    rc = 8

        else:
            msg = "Command : MstatQuery - no such keyword %s" % ty
            State.add_error_msg(msg)
            d6_rc = 7                
        
        # --- this response needs to be in a list
        #print "l is %s" % (l)
        r = Response.GetMstat(return_code=str(rc),
                              cplane_return_code=str(d6_rc),
                              list=l)

        log.debug("Command: ExecuteQuery Exit with %s" %r)

        return str(r)    

###################### undocumented test ##############################
''' this command tests out functions for c-plane program '''
class TestCommand(Command):
    PARAMS = [
        'slot'
        ]

    def execute(self):
        log.debug('Command:test Entered: with %s' % self.slot)
        l = []
        slot = self.slot
        islot = int(slot)

        #log.debug("Command : updating the meta-data on sg module in slot %d" % islot)
        #rv = module_libs.update_sg(islot)
        #if rv:
        #    log.warning("Command : The update of the meta-data on sg module in slot %d was not a success" % islot)
        #else:
        #    log.debug("Command : The update of the meta-data on sg module in slot %d was a success" % islot)

        dev_check = Utils.si_start(islot)
        mount_dev = State.slot_info[islot][dev_check]['dev']
        
        log.debug("module_lib : returning contents of the group %s " % slot)
        # --- IS THE SLOT DISK 0 META DATA PARTITION MOUNTED
        
        if not module_libs.is_mounted(mount_dev):
            # --- IF IT IS NOT, MOUNT IT TO THE APPROPRIATE MOUNT POINT
            module_libs.mount_module(mount_dev, slot, '0', '0')

        rv = 1
        r = Response.SetTest(return_code='0',
                            cplane_return_code=str(rv),
                            list=l)

        log.debug("Command: TestCommand Exit with %s" %r)

        return str(r)    
