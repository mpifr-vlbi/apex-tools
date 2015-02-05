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


import time
import os
import logging
import Utils
import State

log = logging.getLogger('M6')

class persistent:
    global log
    
    def __init__(self):
        global serial
        print "__init__: Persistent State initialization"
        log.debug("__init__: Persistent State initialization")
        if(os.path.isfile("/srv/M6/m6_config.xml")):
            file = open("/srv/M6/m6_config.xml")

        else:
            log.debug("__init__: State creating file /srv/M6/m6_config.xml")
            
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
                  1)
    
        log.debug("State.off_record_sess: d-plane-comm ")
        rv = Utils.d_plane_comm(0 ,"record", rec)
        log.debug("State.off_record_sess: d_plane_comm returned %d " % rv)
        log.debug("State.off_record_sess: leaving ")
        
        return rv 
    #record_session = dict()
    
    def get_record_session():
        global record_session
        return record_session

    def m6_net_interfaces():
        
        log.debug("State.m6_sys_info: Enter")    

        net_int = {}

        # --- GET LIST OF ACTIVE INTERFACES
        active_devs = netinfo.list_active_devs()

        # --- GET LIST OF All INTERFACES
        devs = netinfo.list_devs()

        # --- IF THE NUMBER OF INTERFACES ARE THE SAME, ALL ARE CONFIGURED
        if (len(active_devs)) != (len(devs)):
            log.debug("State.m6_sys_info: All interfaces are not configure")
        print devs

        for dev in devs:
            log.debug("State.m6_sys_info: %s being processed" % dev)
            if not(dev == "lo"):
                if dev in active_devs:
                    log.debug("State.m6_sys_info: %s is in active list" % dev)
                    state ="up"
                    ip = netinfo.get_ip(dev)
                    log.debug("State.m6_sys_info: ip is %s" % ip)                
                    log.debug("State.m6_sys_info: state is %s" % state)
                else:
                    state = "down"
                    ip = "-"
                    log.debug("State.m6_sys_info: ip is %s" % ip)                
                    log.debug("State.m6_sys_info: state is %s" % state)

                log.debug("State.m6_sys_info: state is %s" % state)
                log.debug("State.m6_sys_info: ip is %s" % ip)                
                # --- NEED TO GET THE SPEED OF THE INTERFACES
                log.debug("State.m6_sys_info: calling netinfo.speed with -%s-" % dev)
                (rv, speed) = netinfo.speed(dev)
                log.debug("State.m6_sys_info: returned with rv %d speed is %d" % (rv, speed))

                # --- speed = 
                net_int[dev] = {'state':state, 'ip':ip, 'speed':str(speed)}
                print net_int.keys()

            else:
                log.debug("State.m6_sys_info: %s is skipped" % dev)

        log.debug("State.m6_sys_info: Exit")    
        return(net_int)
        
    # --- module_state() returns the Modules that are available
    def module_list():
        global msn_state
        global slot_info

        # --- number of module enteries 
        lmsn = len(msn_state)
        lslot = len(slot_info)

        # --- if there are no entries, determine if any are available
        if lmsn == 0 or lslot == 0:
            log.debug("State:module_list has no modules available")
            log.debug("State:module_list looking for powered up modules")
            # --- call util.disks to see if any disks are available
            msn_state = Utils.disks()
            log.debug("State:module_list msn_state keys are %s " % (msn_state.keys()))
            
            # --- ASSIGN THE DISK TO THE SPECIFIC SLOT THEY ARE IN:
            for slot, disks in msn_state.iteritems():
                if slot == 0:
                    for i in disks:
                        print "slot %d disk %i" %(slot,i)
                        slot_info[slot] = {}
                        print msn_state[slot][i]
                        if i < 8:
                            print "before assign"
                            slot_info[0][i] = msn_state[slot][i]
                            print "after assign"
                        else:
                            slot_info[1][i] = msn_state[slot][i]                        
                else:
                    for i in disks:
                        print "slot %d disk %i" %(slot,i)
                        if i < 8:
                            slot_info[2][i] = msn_state[slot][i]
                        else:
                            slot_info[3][i] = msn_state[slot][i]                        

            print "slot_info[0] is:", slot_info[0]
            print "slot_info[1] is:", slot_info[1]
            print "slot_info[2] is:", slot_info[2]
            print "slot_info[3] is:", slot_info[3]
        
            rv = len(slot_info[0]) + len(slot_info[1]) + len(slot_info[2]) + len(slot_info[3])

    #--- ELSE LOOK TO SEE IF ANY MODULES ARE POWERED UP
        else:
            log.debug("State:module_list has modules available")
            rv = len(slot_info[0]) + len(slot_info[1]) + len(slot_info[2]) + len(slot_info[3])        

        print " returning from module_list"
        return rv 

    # --- msn_info() gets the info_type for all MSN, or a specific MSN
    def module_info(itype, slot_num):
        global msn_state   # --- CONTAINS ALL OF THE DISKS IN THE SYSTEM
        global slot_info   # --- CONTAINS THE DISKS LOCATED IN THE SPECIFIC SLOT
        msndict = {}

        rv = 1

        log.debug("State.module_info(): for slot_num %d, type = %s "%(slot_num, itype))
        #msndict = Utils.disks()
        module_list()
        log.debug("State.module_info() module_list() returned")
        # --- IF SLOT IS -1, THEN GET ALL
        if slot_num == -1:
            num_disks_in_slot = len(slot_info[0]) + len(slot_info[1]) + len(slot_info[2]) + len(slot_info[3])        
        else:
            num_disks_in_slot = len(slot_info[slot_num])
            print "slot_info is  ",slot_info[slot_num]

        log.debug("State.module_info() : number of disks in slot %d is %d", slot_num, num_disks_in_slot)

        # --- CHET THINK ABOUT HOW TO STORE AND RETURN THIS INFORMATION.  SHOULD IT BE ON EACH DISK DRIVE
        # --- INITIALIZED DURING MOD_INIT?
        if num_disks_in_slot == 0:
            msndict[itype] = "-"
            return msndict
        else:
            log.debug("information in slot_num %d exists" % (slot_num))

        if itype in ["usage", "serial", "model", "size", "temp"]:
            log.debug("State:msn_info() getting %s" % (itype))
            msndict = disk_info(itype, slot_num)
            print "state msndict is :", msndict
        else :
            msg = "State:msn_info() illegal type"
            add_err_msg(msg)
            rv = 0

        return msndict
    
    # --- TERMINATE D-PLANE
    def dterm():
        global seqno
        global command_tag
        log.debug("State.dterm ")    

        rec = create_string_buffer(8)
        log.debug("State:dterm -created 8 byte message")
        pack_into('II',rec, 0,
                  command_tag['terminate'],
                  seqno
                  )
        log.debug("State:dterm - added %d, seqno %d" % (command_tag['terminate'], seqno))
        log.debug("State:dterm - send info to d-plane")
        rv = Utils.d_plane_comm(0 ,"terminate", rec)
        log.debug("State.:dterm - d-plane-comm returned %d " % rv)
        return(rv)

    # --- GROUP FUNCTION FOR MANAGING MODULESD-PLANE
    def group(act, param):
        
        log.debug("State.group - Entered with %s, %d" % (act, param))    
    
        if act == 'open':
            log.debug("State.group - open a group_ref %d for recording" % (param))    
            rv = {'rv':'2', 'gref':param}
            
        elif act == 'auto':
            log.debug("State.group - create group from %d available modules " % (param))    
            rv = {'rv':'2', 'gref':param}

        elif act == 'new':
            log.debug("State.group - create new group from specified modules  %d " % (param))    
            rv = {'rv':'2', 'gref':param}
            
        elif act == 'close':
            log.debug("State.group - close group_ref %d " % (param))    
            rv = {'rv':'2', 'gref':param}

        elif act == 'protect':
            log.debug("State.group - protect group_ref %d" % (param))    
            rv = {'rv':'2', 'gref':param}
            
        elif act == 'unprotect':
            log.debug("State.group - write enable group_ref %d " % (param))    
            rv = {'rv':'2', 'gref':param}
            
        elif act == 'erase':
            log.debug("State.group - erase all of group_ref %d " % (param))    
            rv = {'rv':'2', 'gref':param}

        elif act == 'dismount':
            log.debug("State.group - dismount group_ref %d " % (param))    
            rv = {'rv':'2', 'gref':param}

    # --- 

        return(rv)

##############################################3
    def set_gsm_mask(eme, pme, rme):
        
        log.info("State:set_gsm_mask - setting the gsm_mask")
        return 0

    def get_gsm_mask():
        
        log.info("CState:get_gsm_mask - getting the gsm_mask")

        rv = 0
        return rv

##############################################3

# RETURN SPECIFIC USAGE INFO FOR TYPE IN SLOT   
    def disk_info(infotype, num):
        global slot_info
        rlist = {}
    
        log.debug("disk_info(%s, %d)" % (infotype, num))
        # --- IF SLOT IS 0,2 DISKS 0-7
        if num == -1:
            for i in range (0,4):
                if len(slot_info[i]) != 0:
                    print "disks in slot ->", i
                    rlist[i] = []
                    log.debug("disk_info(%s, %d) rlist[%d] " % (infotype, num, i))            
                    if i % 2:
                        for j in range (8,16):
                            print slot_info[i][j][infotype]
                            rlist[i].append(slot_info[i][j][infotype])
                            log.debug("disk_info(%s, %d) rlist[%d] is %s" % (infotype, num, i, rlist[i]))            
                            print rlist[i]
                    else:
                        for j in range (0,8):
                            print slot_info[i][j][infotype]
                            rlist[i].append(slot_info[i][j][infotype])
                            log.debug("disk_info(%s, %d) rlist[%d] is %s" % (infotype, num, i, rlist[i]))            
                            print rlist[i]
                else:
                    print "no disks in slot ->", i

            return rlist
        
        else:
            rlist[num] = []
            print type(num)
            if num % 2:
                log.debug("disk_info(%s, %d) rlist[%d]  " % (infotype, num, num))            
                for j in range (8,16):
                    print "(%d, %d)" % (num,j)
                    print slot_info[num][j]
                    rlist[num].append(slot_info[num][j][infotype])
                    log.debug("disk_info(%s, %d) rlist[%d] is %s" % (infotype, num, num, rlist[num]))            
                    print rlist[num]        
            else:
                log.debug("disk_info(%s, %d) rlist[%d] " % (infotype, num, num))            
                for j in range (0,8):        
                    print "(%d, %d)" % (num,j)
                    print slot_info[num][j]
                    rlist[num].append(slot_info[num][j][infotype])
                    log.debug("disk_info(%s, %d) rlist[%d] is %s" % (infotype, num, num, rlist[num]))            
                    print rlist[num]        
            
            return rlist
        
