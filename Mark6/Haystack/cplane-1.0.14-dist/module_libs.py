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
import subprocess
import time
import os
import logging
from ctypes import *
from struct import *
import binascii
import Utils
import State
import string
import re 
from glob import glob
import ast
from calendar import timegm
import shutil

log = logging.getLogger('M6')

################################################################################
''' this routing looks at the slot state, if it is initialized properly.  If
the mod_init was executed, get the MSN from the module.  IF THE 
MODULE IS NOT INITIZED RETURN 0, AND A MSN OF "na".
(rv, disk_msn) = module_config(slot_num)
'''
def module_config(sn):
    log.debug("module_config(%d), determine state of module" % sn)
    #print"module_config(%d), determine state of module"% (sn)

    # --- HAS THE MODULE BEEN PARTITIONED PROPERLY
    nd = State.module_list()
    # --- IF NO DISKS PRESENT, RETURN 0
    if nd == 0:
        rv = [0, 'NA']
        return rv
    else:
        # --- DOES THE SLOT REQUESTED HAVE DISKS. number of disks in slot
        nd_ins = len(State.slot_info[sn])
        if nd_ins > 0:
            dev = []
            capacity = []
            total = 0
            tdisks = {}

            # --- GET THE DEVICES AND CAPACITY
            for i in State.slot_info[sn]:
                dev.append(State.slot_info[sn][i]['dev'])
                capacity.append(State.slot_info[sn][i]['size'])
                total = total +  int(State.slot_info[sn][i]['size'])
                log.debug("module_lib - dev is %s" % dev)

            log.debug("module_config - total capacity is %d" %  total)

            # --- SET THE ERROR FLAG TO NULL, IF ONE DISK PARTION HAS AN ERROR, RETURN [0,'NA']
            err_flag = 0
            # --- USER DIRECTORY COUNT, MUST MATCH THE NUMBER OF DISKS
            ud_cnt = 0

            # --- DETERMINE IF THE DISKS HAVE BEEN PARTITIONED
            for d in dev:
                # --- GET THE PARTED INFORMATION FOR THE DEVICE "d" 
                res = subprocess.Popen(["parted", "-s", d, "print"],  stdout=subprocess.PIPE)
                out = res.communicate()[0]
                # --- SPLIT THE LINE ON <CR>
                lines = string.split(out, "\n")
                num_flag = 0
                # --- FOR EACH OUTPUT LINE OF PARTED
                for l in lines:
                    lc = string.split(l)
                    # --- BECAUSE YOU CAN HAVE SPACES IN THE OUTPUT, CHECK THE LENGHT OF DICT IS > 0.
                    if len(lc) > 0:
                        if "Error:" in lc:
                            log.debug("Error -> %s " % l)
                            err_flag = 1
                        elif lc[0] == 'Number':
                            num_flag = 1
                        elif num_flag:
                            if lc[0] in ['1', '2']:
                                # --- GET THE PARTIION NUMBER, START, END, SIZE, FS INFORMATION 
                                log.debug("module_libs - partion %s has size %s fs type %s" % (lc[0], lc[3], lc[4]))
                                # --- IF THE FIRST PARTION IS 10.5MB AND NOT MOUNTED, MOUNT IT
                                if lc[3] == "100MB":
                                    log.debug("module_libs - user directory partitioned")
                                    tdisks[ud_cnt] = [d, lc[4]]
                                    ud_cnt+=1
                                else:
                                    capacity.append( lc[3] )
                                    log.debug("module_libs - capacity is %s" % capacity)
                            else:
                                num_flag = 0
                    #print "ca"

        # --- NO DISKS IN SLOT RETURN 0
        else:
            rv = [0, 'NA']
            return rv    

    #print "tdisks is -> ", tdisks

    mnt_flag = 0

    if err_flag == 0 and ud_cnt == nd_ins:
        log.debug("module_libs - module proper configuration, get eMSN and other information")
        # --- MOUNT ALL THE USER DIRECTORIES
        cnt = 0
        for d in dev:
            # --- dev is partion1, so change d to /dev/sdX1
            d = d + "1"
            log.debug("module_libs - mount %s under /mnt/disks/.meta/%s/%d type %s" % (d, sn, cnt, tdisks[cnt][1]))
            mtpt =  "/mnt/disks/.meta/%s/%d" % (sn, cnt)

            res =  subprocess.Popen(["grep", mtpt, "/etc/mtab"],  stdout=subprocess.PIPE)
            out = res.communicate()[0]  
            # --- SPLIT THE LINE ON <CR>
            lines = string.split(out)
            #print lines

            if len(lines) > 0:
                if mtpt in lines:
                    log.warning("module_libs - %s is already mounted", mtpt)
                    mnt_flag = 1

            if not mnt_flag:
                # --- IF SLOT AND DISK MSI ALREADY MOUNTED, ERROR
                res = subprocess.Popen(["sudo", "/bin/mount", "-t", tdisks[cnt][1], '-o', "grpid", d, mtpt],  stdout=subprocess.PIPE)
                out = res.communicate()[0]  
                #print out
            else:
                mnt_flag = 0

            cnt += 1

        # --- GET THE MODULE INFORMATION
    else:
        msg = "module_libs - disks in module NOT properly configured"
        State.add_error_msg(msg)

    # --- READ 

    # --- IF YES, MOUNT THE USER_DIRECTORY
    #yes = "no"
    #if yes = "yes":
        # --- GET THE MSN, AND OTHER MODULE INFORMATION
    #    log.debug("module_config(%d), msn is %s"%(sn, mc_MSN))
    #else:
    #    log.debug("No module_config(%d), msn is %s"%(sn, mc_MSN))

#####################################################
def modules(slot_list):
    rl = []
    rv = 0

    num_sl = len(slot_list)
    #print "num_sl - ", num_sl
    log.debug("modules_lib - entered with slot_list of %s " %  slot_list)

    if num_sl > 0:
        for i in slot_list:
            log.debug("module_libs - slot %d, num disks %s, capacity %s " %  (i, State.slot_status[i]['disks'],  State.slot_status[i]['cap']))
            if i in State.slot_info.keys():
                l_si = len(State.slot_info[i])
                #print "slot_info is ", State.slot_info[i]
            else:
                l_si = 0
                #print "len", l_si

            # --- IF THE NO. OF DISKS STARTED IN SLOT INFO IS 0, BUT SLOT_INFO IS NOT, UPDATE MAIN VALUE
            if State.slot_status[i]['disks'] == 0 and l_si > 0:
                State.slot_status[i]['disks'] = l_si
                log.debug("module_libs - setting  State.slot_status[%d]['disks'] from 0 to %d" % (i, l_si))
            else:
                log.debug("module_libs - State.slot_status[i]['disks'] != 0 and len of slot info is not > 0")
                          
            # --- IF THE DISK FOUND IS > 0 and capacity is 0
            if (State.slot_status[i]['disks'] > 0) and ( State.slot_status[i]['cap'] == 0):
                log.debug("module_libs - disks found calculate the capacity ")
                # --- get the capacity from all the individual disks
                #for j in range (int(State.slot_status[i]['disks'])):
                for j in State.slot_info[i].keys():
                    log.debug("module_libs - for disk %d " % j)
                    #print "keys available are ", State.slot_info[i].keys()
                    State.slot_status[i]['cap'] += (int(State.slot_info[i][j]['size']))/1000000000
                    log.debug("module_libs - disk %d capacity is %d " % (j, State.slot_status[i]['cap']))

                    if State.slot_info[i][j]['usage'] == 'sg' or  State.slot_info[i][j]['usage'] == 'raid':
                        State.slot_status[i]['rdisks'] += 1
                        State.slot_status[i]['type'] =  State.slot_info[i][j]['usage']
                        log.debug("module_libs: updating slot_status[%d]['type'] to %s " % (i, State.slot_status[i]['type']))

                    log.debug("module_libs - number of registered disks are %d " % State.slot_status[i]['rdisks'])

                # --- IF IT IS A RAID, DETERMINE IF IT IS IN MDSTAT.  IF IT IS, IT IS CLOSED
                if  State.slot_status[i]['type'] == 'raid':
                    if(raid_dev_check(i) != "-"):
                        State.slot_status[i]['status1'] = "closed"
                        State.slot_status[i]['status2'] = "unprotected"
                    else:
                        # --- since there are registered disks, change the status to initialized
                        State.slot_status[i]['status1'] = "initialized"
                        # --- RAIDs of for one module only, so set the group to slot

                    State.slot_status[i]['group'] = str(i)
 
                # --- ELSE THEY ARE SG DISKS AND SHOULD BE INITIALIZED.  HAVE TO CHECK GROUP TO SEE IF CLOSED / INCOMPLETE
                elif  State.slot_status[i]['type'] == 'sg':
                    State.slot_status[i]['status1'] = "initialized"                    

                else:
                    State.slot_status[i]['status1'] = "unknown"                    

            # --- IF THE MSN IS NOT INITIALIZED, SEE IF IT IS AVAILABLE
            if State.slot_status[i]['eMSN'] == "-":
                msn = get_eMSN(i,State.slot_status[i]['type'])
                log.debug("Module_libs : msn was not initialized, got %s for slot %d " % (msn, i))
                # --- IF IT IS THE MSN AND NOT EMSN, CREATE AND UPDATE
                if msn.find("/") == -1 and msn != "-" :
                    eMSN = set_eMSN(msn, i,State.slot_status[i]['cap'] , 4, State.slot_status[i]['rdisks'],  State.slot_info[i][j]['usage'])
                    log.debug("module_libs : eMSN is set to %s for slot %d " % (eMSN, i))
                else:
                    eMSN = msn

                State.slot_status[i]['eMSN'] = eMSN

            rl.append([State.slot_status[i]['group'],
                       str(i),            
                       State.slot_status[i]['eMSN'],
                       str(State.slot_status[i]['disks']),
                       str(State.slot_status[i]['rdisks']),
                       str(State.slot_status[i]['rem']),
                       str(State.slot_status[i]['cap']),
                       State.slot_status[i]['status1'],
                       State.slot_status[i]['status2'],
                       State.slot_status[i]['type']])

    else:
            rl.append(['-',
                       '0',            
                       '-',
                       '-',
                       '-',
                       '-',
                       '-',
                       '-',
                       '-',])

    #print "rl is %s" %(rl)

    return (rv, rl)

############################################################
def slot_mnt():
    raid_fl = 0

    r = subprocess.Popen(["df", "-h"],stdout=subprocess.PIPE)
    o = r.communicate()[0]

    l = string.split(o, "\n")
    # --- FOR EACH LINE IN DF
    for i in l:
        # --- LOOK FOR MNT, WHERE ALL MODULES WOULD BE.
        if "mnt" in i:
            if "md" in i:
                log.info("module_libs - the module in slot %d is mounted as a raid" % i)
                r2 = subprocess.Popen(["cat", "/proc/mdstat"],stdout=subprocess.PIPE)
                o2 = r2.communicate()[0]
                
                l2 = string.split(o2, "\n")
                # --- GET DEVICES AND STATE AND PERSONALITY
                for j in l2:
                    if "Personalities" in j:
                        l3 = string.split(j, ":")
                        if "raid0" in l3[1]:
                            log.info("module_lib - Configured as raid 0 ")
                            raid_fl = 1
                        else:
                            log.info("module_lib - invalid raid configuration %s" % l3)
                    elif "md" in j:
                        l3 = string.split(j)
                        log.debug("mdx is here %s " % l3)
                        for k in l3:
                            log.info("module_lib - k is %s " % k)
                            if "sd" in k:
                                log.debug("module_lib - disk in raid is %s " % k)


##################################################################################3
''' this routine initializes a module to a xfs format for scatter / gather
    usage.  If this is called, it will erase the disks and set the label
'''
def sg_init(islot, msn, num_disks):
    log.debug("creating a sg module in slot %d, with MSN %s for %d disks" %(islot, msn, num_disks))

    # --- GET THE DISKS FOR ISLOT
    if islot in State.slot_info.keys():
        l_si = len(State.slot_info[islot])
        if l_si != num_disks:
            msg = "module_lib : error, the number of disks expected %d are not found %d" % (num_disks, l_si)
            State.add_error_msg(msg)
            return ('0',  '8', '-')
        else:
            log.debug("module_lib : have information on the disks, continuing")
            disk = State.slot_info[islot]
    else:
        # --- IF there are no keys for slot, re-read contents
        msg = "error, there is no information on slot  %d rereading disk info" % (islot)
        State.add_error_msg(msg)
            

    log.debug("module_lib :Slot %d -> %s" % (islot, disk))
    #print "information", disk
    dev_start= Utils.si_start(islot)

    log.debug("module_libs : starting device number is %d" % (dev_start))
    dev_end = dev_start + num_disks
    log.debug("module_libs : ending device number is %d" % (dev_end))
    # --- FOR ALL DISKS IN NUM_DISKS
    #for i in range (0,l_si):
    
    mpt_loc = 0    # --- COUNTER FOR MOUNT LOCATION BASED ON SLOT
    for i in range (dev_start,dev_end):
        
        # --- GET THE DEVICE YOU ARE PERFORMING INIT ON:
        dev = disk[i]['dev']
        log.debug("dev is %s" % dev)
        # --- IF LOCATION IS TRUE, A DIGIT IS FOUND
        loc = re.search(r"(\d+)", dev)
        if loc == None:
            log.debug("module_libs : no digit was found for device, %s" % dev)
        else:
            dev = dev[:dev.find(loc.group())]
            log.debug("module_libs : device is now %s" % dev)
        # --- IF THE DISKS HAS PARTITIONS, REMOVE THEM BEFORE CONTINUEING
        ipc = int(State.slot_info[islot][i]['count'])
        #print "ipc ", ipc
        if ipc > 0:
            log.debug("module_libs : removing %d partitions from disk before formatting " % ipc)
            for j in range (1, ipc+1):
                log.debug("remove partition %d" % j)
                # --- ERASE THE DISK AND CREATE GPT PARTITION
                cmd_to_ex = ["sudo", "/sbin/parted", '-s', dev, 'rm', str(j)]
                (rv, msg) = Utils.linux_cmd(cmd_to_ex)
                if not rv:
                    State.add_error_msg(msg)
                    rv &= 0

        # --- ERASE THE DISK AND CREATE GPT PARTITION
        cmd_to_ex = ["sudo", "/sbin/parted", '-s', dev, 'mktable', 'gpt']                    
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0

        # --- CREATE THE LABEL
        base_label = msn+'_'
        log.debug("module_libs : base_label = %s" % base_label)

        # --- LABALE DATA PARTITION
        l1 = base_label+str(i%8)
        log.debug("module_libs : label of 1st partition %s" % (l1))
        # --- SET UP THE FIRST PARTITION
        #sudo -s dev 'mkpart primary 1 -100M' mkname 1 
        cmd_to_ex = ["sudo", "/sbin/parted", '-s', dev, 'mkpart primary xfs 1 -100M']
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0

        name = 'name 1 ' + l1
        log.debug("module_libs : write label to disk using %s " % name)
        cmd_to_ex = ["sudo", "/sbin/parted", '-s', dev, name]
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0

        # --- LABEL meta DATA PARTITION
        l2 = l1+'m'
        #print "label of 2nd partition %s" % (l2)
        # --- SET UP THE 2nd  PARTITION
        #sudo -s dev 'mkpart primary xfs-100M 100%' mkname 2 l2
        cmd_to_ex = ["sudo", "/sbin/parted", '-s', dev, 'mkpart primary xfs -100M 100%']
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0

        name = 'name 2 ' + l2
        cmd_to_ex = ["sudo", "/sbin/parted", '-s', dev, name]
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0

        # --- FORMAT THE PARTITION
        d1 = dev+'1'
        log.debug("module_libs : Format device %s" % (d1))
        
        cmd_to_ex = ["sudo", "/sbin/mkfs.xfs",  '-f', d1]
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0


        # --- FORMAT THE PARTITION
        d2 = dev+'2'
        log.debug("module_libs : Formatting device %s" % (d2))
        #mkfs.xfs d2
        cmd_to_ex = ["sudo", "/sbin/mkfs.xfs", '-f' ,d2]
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0

        # --- MOUNT THE META DATA AREA
        meta_loc = State.mnt_meta + str(islot) +'/'+ str(mpt_loc)
        log.debug("module_libs : device to mount is %s at %s" % (d2, meta_loc))
        cmd_to_ex = ["sudo", "/bin/mount", "-t", "xfs", d2, meta_loc]
        #["sudo", "/bin/mount", "-t", "xfs", "-o", 'grpid', d2, meta_loc]
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0

        cmd_to_ex = ["sudo", "chmod", "775", meta_loc]
        log.debug("module_lib : send to execute %s " % cmd_to_ex)
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0

        cmd_to_ex = ["sudo", "chown", "root.mark6", meta_loc]
        log.debug("module_lib : send to execute %s " % cmd_to_ex)
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0

        mpt_loc += 1

    # --- CALL module_init to reset the settings
    #print "calling modules"
    tmp = State.module_list()
    #print "init_sg, returned module_list to update sas_device information ", tmp

    isl = [islot]
    (rv, rl) = modules(isl)

    # --- GET THE MODULES CAPACITY
    cap = State.slot_status[islot]['cap']
    #print "cap is ", cap

    return ('0', l_si, cap)
##################################################################################3
''' this routine initializes a module to a XFS type for RAID 0 usage.  
'''
def raid_zero_init(islot, msn, num_disks):
    log.debug("module_libs : creating a RAID0 module in slot %d, with MSN %s for %d disks" %(islot, msn, num_disks))
    rv = 1

    # --- RAID DEV LIST
    dr = []
    # --- GET THE DISKS FOR ISLOT
    if islot in State.slot_info.keys():
        l_si = len(State.slot_info[islot])
        if l_si != num_disks:
            msg = "raid_zero_init : error, the number of disks expected %d are not found %d" % (num_disks, l_si)
            State.add_error_msg(msg)
            return 0
        else:
            log.debug("module_libs : have information on the disks, continuing")
            disk = State.slot_info[islot]
    else:
        # --- IF there are no keys for slot, re-read contents
        log.info("module_libs : there is no information on slot  %d rereading disk info" % (islot))
        
    #print "Slot %d -> %s" % (islot, disk)
    #print "information", disk
        
    # --- FOR ALL DISKS IN NUM_DISKS
    #for i in range (0,l_si):
    for i in State.slot_info[islot].keys():

        # --- GET THE DEVICE YOU ARE PERFORMING INIT ON:
        dev = disk[i]['dev']
        #print "dev is ", dev
        loc = re.search('[0-9]', dev)
        # --- IF LOCATION IS TRUE, A DIGIT IS FOUND
        if loc:
            dev = dev[:dev.find(loc.group())]
        #    print "device is now ", dev
        else:
            log.debug("module_libs : no digit was found in device %s " %  dev)

        # --- IF THE DISKS HAS PARTIIONS, REMOVE THEM BEFORE CONTINUEING
        ipc = int(State.slot_info[islot][i]['count'])
        if ipc > 0:
            log.debug("module_libs : removing %d partitions from disk before formatting " % ipc)
            for j in range (1, ipc+1):
                log.debug("remove partition %d" % j)
                # --- ERASE THE DISK AND CREATE GPT PARTITION
                cmd_to_ex = ["sudo", "/sbin/parted", '-s', dev, 'rm', str(j)]
                log.debug("module_lib : send to execute %s " % cmd_to_ex)
                (rv, msg) = Utils.linux_cmd(cmd_to_ex)
                if not rv:
                    State.add_error_msg(msg)
                    rv &= 0

        # --- ERASE THE DISK AND CREATE GPT PARTITION
        cmd_to_ex = ["sudo", "/sbin/parted", '-s', dev, 'mktable', 'gpt']
        log.debug("module_lib : send to execute %s " % cmd_to_ex)
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0

        # --- CREATE THE LABEL
        base_label = msn+'_'

        log.debug("module_libs : base_label = %s" % base_label)

        # --- LABALE DATA PARTITION
        l1 = base_label+str(i%8)
        #print "label of 1st partition %s" % (l1)
        # --- SET UP THE FIRST PARTITION
        #sudo -s dev 'mkpart primary 1 -100M' mkname 1 
        cmd_to_ex = ["sudo", "/sbin/parted", '-s', dev, 'mkpart primary xfs 1 100%']
        log.debug("module_lib : send to execute %s " % cmd_to_ex)
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0
        
        name = 'name 1 ' + l1
        log.debug("module_libs :  write label to disk using %s" % name)
        cmd_to_ex = ["sudo", "/sbin/parted", '-s', dev, name]
        log.debug("module_lib : send to execute %s " % cmd_to_ex)
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0

        # --- FORMAT THE PARTITION
        d1 = dev+'1'
        log.debug("module_libs : device one is %s" % (d1))
        # --- APPEND THE DEVICE
        dr.append(d1)

        #mkfs.xfs d1
        cmd_to_ex = ["sudo", "/sbin/mkfs.xfs",  '-f', d1]
        log.debug("module_lib : send to execute %s " % cmd_to_ex)
        (rv, msg) = Utils.linux_cmd(cmd_to_ex)
        if not rv:
            State.add_error_msg(msg)
            rv &= 0

        # --- CALL module_init to reset the settings.
        
    log.debug("module_libs : devices to use are %s " % dr)
    md_slot = find_mdslot(islot)

    # --- VERIFY NUMBER OF DEVICES IN DR = RAID-DEVICES
    if len(dr) == 8:
        log.debug("module : the device list is of proper length")
    else:
        msg = "number of devices to raid %d does not equal what is expected 8" % len(dr)
        State.add_error_msg(msg)
                 
    # --- wait for the process to finish
    time.sleep(5)
    # --- create the RAID with mdadm WITH X EQUAL TO SLOT
    #mdadm --create /dev/mdx --level 0 --raid-devices=8 /dev/sd........
    log.debug("creating raid for slot %d device %s" % (islot, md_slot))
    cmd_to_ex = ["sudo", "/sbin/mdadm", '--create', md_slot, '--level=0', '--raid-devices=8', dr[0], dr[1], dr[2], dr[3], dr[4], dr[5], dr[6], dr[7]]
    log.debug("module_lib : send to execute %s " % cmd_to_ex)
    (rv, msg) = Utils.linux_cmd(cmd_to_ex)
    if not rv:
        State.add_error_msg(msg)
        rv &= 0

    log.debug("device created raid for slot %d device %s" % (islot, md_slot))
    # --- ASSIGN THE RAID SLOT
    State.slot_raid_mnt[islot]['type'] = 'raid'
    State.slot_raid_mnt[islot]['info'] = {}
    State.slot_raid_mnt[islot]['info']['dev'] = md_slot
    State.slot_raid_mnt[islot]['info']['mnt'] = '-'


    #print "State.slot_raid_mnt[%d] %s" % (islot, State.slot_raid_mnt[islot])
    time.sleep(3)

    log.debug("format device device %s" % (md_slot))
    res = subprocess.Popen(["sudo", "/sbin/mkfs.xfs",  '-f', md_slot],  stdout=subprocess.PIPE)
    out = res.communicate()[0]  
    #print "out is ", out
    log.debug("device %s formatted " % (md_slot))        

    # --- update module information
    tmp = State.module_list()
    #print "init_sg, returned module_list to update sas_device information ", tmp
    isl = [islot]
    (rv, rl) = modules(isl)

    # --- recalculate slot instead of hard coded
    cap = State.slot_status[islot]['cap']

    return ('0', l_si, cap)


###############################################################################################3
def find_mdslot(islot):
    found = 0
    # --- DETERMINE IF THE /DEV/MD(SLOT) EXISTS
    md = "md"+str(islot)
    md_slot = '/dev/' + md
    log.debug("module_libs : device to create RAID mount point is %s " % md_slot)
    
    if os.path.exists(md_slot):
        log.debug("module_libs : %s exists" % md_slot)
        if (os.path.isfile("/proc/mdstat")):
            log.debug("module_libs : /proc/mdstat exists")
        
            # --- DETEREMINE IF IT IS IN /PROC/MDSTAT, IF NOT ASSIGN IT, IF YES, THEN FIND ANOTHER SLOT
            res =  subprocess.Popen(["grep", md, "/proc/mdstat"],  stdout=subprocess.PIPE)
            out = res.communicate()[0]  
            # --- SPLIT THE LINE ON <CR>
            lines = string.split(out)

            if len(lines) > 0:
                log.debug("module_libs : process lines")
                found = 1
            else:
                log.debug("module_libs : %s does not exist in mdstat" % md)

        if found == 1:
            while os.path.exists(md_slot):
                log.debug("module_libs : raid device exists %s find another" % md_slot)
                next_slot = islot + 1
                md_slot = '/dev/md'+str(next_slot)
            else:
                log.debug("module_libs : raid device %s does not exist continue" % md_slot)
   
    log.debug("module_libs : returns %s " % md_slot)
    return md_slot

##################################################################################3
''' this routine initializes a modules eMSN IN THE FORMAT INS_SERIAL/SIZE/SPEED/DISK_TYPE.  
'''
def set_eMSN(msn, islot, cap, speed, dt, ft):
    log.debug("creating the eMSN for slot %d from MSN of %s with cap %d" %(islot, msn, cap))
    # --- CAPACITY IS IN TB, check to see what size was passed, if 0 then write cap, if not recalculate
    if cap / 1000000000000:
        cap /= 1000000000000

    #print "cap is ", cap

    eMSN = msn + '/' + str(cap) + '/' + str(speed) + '/' + str(dt)

    if ft == 'sg':
        dest_dir = State.mnt_meta + str(islot)
        # --- SET UP THE START DEST DIR
        
        for i in range (int(dt)): 
            dest_file = dest_dir + '/' +str(i) + "/eMSN"
            if(os.path.isfile(dest_file)):
                emsn_file = open(dest_file, 'w')
                emsn_file.write(eMSN)
                log.debug("module_libs : updated the eMSN %s to meta data %s location" % (dest_file, eMSN))
                emsn_file.close()
            else:
                log.info("module_libs : cannot update the eMSN %s the meta data %s location does not exist" % (dest_file, eMSN))
        log.debug("module_libs : eMSN is %s :" % eMSN)
    # --- ELSE IF IT IS A RAID, WAIT TILL IT IS MOUNTED TO WRITE THE META DATA.
    elif ft == 'raid':
        dest_file = State.raid_meta + "/eMSN"
        if(os.path.isfile(dest_file)):
            emsn_file = open(dest_file, 'w')
            emsn_file.write(eMSN)
            log.debug("module_libs : updated the eMSN %s to meta data %s location" % (dest_file, eMSN))
            emsn_file.close() 

    return eMSN

##################################################################################3
''' this routine initializes a modules eMSN IN THE FORMAT INS_SERIAL/SIZE/SPEED/DISK_TYPE.  
'''
def get_eMSN(islot, ft):
    log.debug("reading the eMSN for slot %d " %(islot))
    eMSN = "-"

    if ft == 'sg':
        dest_dir = State.mnt_meta +str(islot)+'/'
        # --- SET UP THE START DEST DIR
        dest_file = dest_dir + '0' + "/eMSN"
        log.debug("module_libs : checking for existence of %s" %  dest_file)
        if(os.path.isfile(dest_file)):
            emsn_file = open(dest_file, 'r')
            eMSN = emsn_file.readline()
            log.debug("module_libs : updated the eMSN %s to meta data %s location" % (dest_file, eMSN))
            emsn_file.close()
        else:
            log.warning('module_libs : eMSN was not written to sg disk module, it will be updated later')

    # --- ELSE IF IT IS A RAID, WAIT TILL IT IS MOUNTED TO WRITE THE META DATA.
    elif ft == 'raid':
        dest_file = State.raid_meta + "/eMSN"
        if(os.path.isfile(dest_file)):
            emsn_file = open(dest_file, 'r')
            eMSN = emsn_file.readline()
            log.debug("module_libs : updated the eMSN %s to meta data %s location" % (dest_file, eMSN))
            emsn_file.close()
        else:
            log.warning('module_lib : eMSN was not found for RAID, the group may not be mounted, or it was not initialized properly, please verify ')
    else:
        log.info('module_lib : no module found in %d therefore eMSN was not found ' % islot)        

    log.debug("module_libs : initial eMSN  is set to %s" % eMSN)

    if eMSN == "-":
        # --- GET LABEL FROM SLOT_INFO 
        # --- DEV_NUM IS SINCE SLOT INFO IS 1 AND 3 HAVE 0-7, 2 AND 4 8-15 
        dev_num = Utils.si_start(islot)

        log.debug("module_libs : dev_num %d, slot %d " % (dev_num, islot))
        # --- INIDIVIDUAL DISK LABEL 
        if  State.slot_info[islot].keys():
            # --- check for dev_num key
            dev_found = 0
            while dev_found == 0:
                if dev_num in State.slot_info[islot]:
                    dev_found = 1
                    if State.slot_info[islot][dev_num].has_key('part_label'):
                        msn_dl = State.slot_info[islot][dev_num]['part_label']
                        log.debug("module_libs : msn read from partition label is %s " % msn_dl)
                        
                        msn = msn_dl[:msn_dl.rfind("_")]
                        dev_found = 1
                        # --- MAY WANT TO CALCULATE MSN HERE
                        eMSN = msn
                    else:
                        log.info("module_libs : No msn for partition label - uninitialized module")

                else:
                    log.warning("module_libs : slot %d, dev %d is not found in module" % (islot, dev_num))
                    # --- INCREMENT TO NEXT DEVICE TO SEE IF IT EXISTS
                    dev_num += 1
                    log.info("Checking next device %d " % dev_num)
                    if dev_num % 8:
                        log.info("no devices found, exiting function")
                        break
        else:
            log.info("module_libs : slot %d, has no detected disks" % (islot))
            

    log.debug("module_libs : eMSN is set to %s" % eMSN)

    return eMSN

#####################################################################################
'''# raid_dev_check takes the slot information and determines if a raid device /dev/mdX
     exist for it.  Used by the assemble process.  It returns "-" or the device
'''
def raid_dev_check(islot):            
    log.debug("module_libs : Checking slot %d to see if a /dev/mdX is assigned to it" % islot)
    # --- INITIALIZE THE RETURN DEVICE UNITIALIZED
    ret_device = '-'

    if os.path.isfile("/proc/mdstat"):

        res = subprocess.Popen(["cat","/proc/mdstat"],  stdout=subprocess.PIPE)
        out = res.communicate()[0]    
        lines = out.split("\n")

        num_dev = len( State.slot_info[islot])
        # --- FIND THE MDX LINE IF IT EXISTS
        for ln in lines:
            if 'md' in ln:
                # --- DEVICE COUNTER
                dev_cnt = 0
                log.debug("moduel_libs : md line is %s" % ln)
                # --- ALL KEYS MUST BE CHECKED AND IN THE RAID LIST
                for i in State.slot_info[islot].keys():
                    ldev_check = State.slot_info[islot][i]['dev']
                    ldev_check = ldev_check.split("/")
                    dev_check = ldev_check[2]
                    log.debug("module_libs : Checking device %s to see if a /dev/mdX is assigned to it" % dev_check)
                    # --- IF DEV IN LINE:
                    if dev_check in ln:
                        dev_cnt+=1
                        log.debug("module_libs : Device %s is in /dev/mdX is assigned to it" % dev_check)           
                    else:
                        log.debug("module_libs : Device %s is NOT in /dev/mdX is assigned to it" % dev_check)

                # --- IF DEV_CNT EQUALS THE NUMBER EXPECTED
                if dev_cnt == num_dev:
                    lln = ln.split(":")
                    rv = lln[0]
                    ret_device = rv.replace(' ', '')
                    log.debug("module_libs : Device %s " % ret_device)
                    return ret_device
        
        # --- NO DEVICES ASSOCIATED WITH DISKS WAS FOUND, 
        log.debug("module_libs : No device exists for these modules in slot %d " % islot)

    else:
        log.info("module_libs : No raid devices exist")
    
    return ret_device

    
#####################################################################################
'''# raid_mnt expects an action = {assemble, unmount, stop, close, mount} for a particular 
     module in the slot provided.  It will call mdadmn and process the command
'''
def raid_mnt(action, slot):
    log.debug("module_libs : perform %s on slot %d configured as a RAID0 " % (action, slot))
    dr = []
    if type(slot) == 'str':
        islot = int(slot)
    else:  islot = slot

    if action == 'mount':
        log.debug("module_libs : We are attempting to mount the module in slot %d" % islot)
        md_slot = raid_dev_check(islot)
        log.debug("module_libs : The module in slot %d is part of assembly %s" % (islot, md_slot))          
        if md_slot == '-':

            log.debug("module_libs : No mdadm devices for slot, assembling RAID0 in slot %d " % islot)
            # --- GET THE SLOT DEVICES
            for i in State.slot_info[islot].keys():
                dr.append(State.slot_info[islot][i]['dev'])

            log.debug("the devices of slot %d are %s " % (islot, dr))
            # --- WILL NEED TO SEE IF THE DEVICES ARE ALLOCATED TO A DEVICE, WILL INITIALLY ASSUME IT IS NOT: 
            ms_slot = find_mdslot(islot)

            log.debug("module_libs : assembling RAID0 in slot %d for device %s  " % (islot, ms_slot))
            # --- ASSEMBLE THEM
            #res = subprocess.Popen(["sudo", "/sbin/mdadm", '--assemble', ms_slot, 
            #                        dr[0], dr[1], dr[2], dr[3], dr[4], dr[5], dr[6], dr[7]],  stdout=subprocess.PIPE)
            #out = res.communicate()[0]



            cmd_to_ex = ["sudo", "/sbin/mdadm", '--assemble', ms_slot, dr[0], dr[1], dr[2], dr[3], dr[4], dr[5], dr[6], dr[7]]
            log.debug("module_lib : send to execute %s " % cmd_to_ex)
            (rv, msg) = Utils.linux_cmd(cmd_to_ex)
            if not rv:
                State.add_error_msg(msg)
                rv &= 0

            log.debug("module_libs - Assembled raid for slot %d device %s" % (islot, md_slot))
            md_slot = raid_dev_check(islot)
            log.debug("module_libs : The module in slot %d is part of assembly %s" % (islot, md_slot))         

        # --- CHECK THE RAID DEVICE, IF IT ASSEMBLED THEN MOUNT IT.
        md_slot = raid_dev_check(islot)

        if md_slot != "-":
            dev = "/dev/"+md_slot
            log.debug("module_libs : device to mount is %s" % dev)
            #res = subprocess.Popen(["sudo", "/bin/mount", "-t", "xfs", '-o','grpid',dev, '/mnt/raid'],  stdout=subprocess.PIPE)
            #out = res.communicate()[0]
            #line = out.split("\n")

            cmd_to_ex = ["sudo", "/bin/mount", "-t", "xfs", '-o','grpid',dev, State.raid_data]
            log.debug("module_lib : send to execute %s " % cmd_to_ex)
            (rv, msg) = Utils.linux_cmd(cmd_to_ex)
            if not rv:
                State.add_error_msg(msg)
                rv &= 0
            
            cmd_to_ex = ["sudo", "chmod", "775", State.raid_data]
            log.debug("module_lib : send to execute %s " % cmd_to_ex)
            (rv, msg) = Utils.linux_cmd(cmd_to_ex)
            if not rv:
                State.add_error_msg(msg)
                rv &= 0

            cmd_to_ex = ["sudo", "chown", "root.mark6", State.raid_data]
            log.debug("module_lib : send to execute %s " % cmd_to_ex)
            (rv, msg) = Utils.linux_cmd(cmd_to_ex)
            if not rv:
                State.add_error_msg(msg)
                rv &= 0

            cmd_to_ex = ["mkdir", State.raid_meta]
            log.debug("module_lib : send to execute %s " % cmd_to_ex)
            (rv, msg) = Utils.linux_cmd(cmd_to_ex)
            if not rv:
                State.add_error_msg(msg)
                rv &= 0
            
            #print "slot_raid_mnt ", State.slot_raid_mnt[islot]
            if not State.slot_raid_mnt[islot].has_key('info'):
                State.slot_raid_mnt[islot]['info'] = {}
                #print "create key info"

            #print "before"
            State.slot_raid_mnt[islot]['info']['dev'] = dev
            #print "dev = ", dev
            State.slot_raid_mnt[islot]['info']['mnt'] = State.raid_data
            #print "mnt = /mnt/raid"
            log.info("module_libs : RAID0 Group in slot %d was mounted at /mnt/raid" % islot)

    elif action == 'stop' or action == 'close':
        m_slot = State.slot_raid_mnt[islot]['info']['mnt']
        # --- UNMOUNT THE DISK
        res = subprocess.Popen(["sudo", "umount", m_slot],  stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        out, err = res.communicate()
        if err != '':
            msg = "umount of %s produced " % (m_slot, err)
            return 0
        else:
            State.slot_raid_mnt[islot]['info']['mnt'] = '-'

    elif action == 'unmount':
        md_slot = raid_dev_check(islot)
        if md_slot == '-':
            md_slot = find_mdslot(islot)

        ms_slot = '/dev/' + md_slot
        log.info("module_libs : unmounting slot %d, device %s" % (islot, ms_slot))
        # --- WILL NEED TO VERIFY THE MODULE IS NOT PART OF AN OPEN GROUP
        res = subprocess.Popen(["sudo", "/sbin/mdadm", '--manage', "--stop", ms_slot],  stdout=subprocess.PIPE)
        
        log.info("module_libs : RAID0 Group %d was stopped, it can be powered off" % islot)

    return 1


###########################################################################
def getremainingspace(group_ref):

    #print "grs ", group_ref
    # --- CONVERT GROUP_REF TO A LIST
    lslots = list(group_ref)
    
    llslots = len(lslots)

    rem = '-'

    # --- IF THERE IS > 1 SLOT BEING USED FOR THE GROUP ASSUME SG MODE
    if State.slot_status[int(lslots[0])]['type'] == 'sg':
            
        res = subprocess.Popen(["df","-t", "xfs"],  stdout=subprocess.PIPE)
        out = res.communicate()[0]
        # --- THE OUTPUT IS SEPARATED BY CR"
        mdf = out.split("\n")

        ldf = len(mdf)
        
        if ldf > 1:
            
            for s in range(llslots):
                tot_avail = 0
                for i in range(1, ldf-1):
                    #print "mdf ", mdf[i]
        
                    (dev, size, used, avail, percent_used, mt_pt) = mdf[i].split()
                    if '1' in dev:
                        #print "found 1"
                        mt_pt = mt_pt.split('/')
                        #print "mt_pt is ", mt_pt
                        #print "lslots is ", lslots[s]                    
                        if mt_pt[3] == lslots[s]:
                            tot_avail += int(avail)
                            #print "tot_avail ", tot_avail

                # --- calculate the remaining storage
                log.debug("module_libs : total available is %d" % (tot_avail))

                # --- must multiple by 1024 to get proper size since in blocks of 1024
                #rem = State.slot_status[int(lslots[s])]['rem'] = (tot_avail * 1024)
                tot_avail *= 1024
                rem = State.slot_status[int(lslots[s])]['rem'] = tot_avail / 1000000000

    elif State.slot_status[int(lslots[0])]['type'] == 'raid':
            
        res = subprocess.Popen(["df","-t", "xfs"],  stdout=subprocess.PIPE)
        out = res.communicate()[0]
        # --- THE OUTPUT IS SEPARATED BY CR"
        mdf = out.split("\n")
            
        ldf = len(mdf)
        tot_avail = 0

        if ldf > 1:

            for i in range(1, ldf-1):
                (dev, size, used, avail, percent_used, mt_pt) = mdf[i].split()
                tot_avail += int(avail)

            # --- calculate the remaining storage
            log.debug("module_libs : total available is %d" % (tot_avail))

            # --- must multiple by 1024 to get proper size since in blocks of 1024
            #rem = State.slot_status[int(lslots[s])]['rem'] = (tot_avail * 1024)
            tot_avail *= 1024
            rem = State.slot_status[int(lslots[0])]['rem'] = tot_avail / 1000000000

    return rem

###########################################################################
''' modules_status checks the state of the group_ref with slot_status and returns
    the status string to be or'd
'''
def modules_status(group_ref):

    # --- CONVERT GROUP_REF TO A LIST.
    lslots = list( group_ref )
    
    llslots = len(lslots)
    # --- INITIALIZE RETURN VALUE
    rv = 0x0

    if llslots > 0:
        # --- CONVERT THE STRING LIST TO INTS
        ilslots = map(int, lslots)
        for i in ilslots:
            log.debug("module_libs : slot %d, status %s " % (i, State.slot_status[i]['status1']))
            if State.slot_status[i]['status1'] == 'open' :
                val = 0x1
                if State.slot_status[i]['status2'] == 'ready':
                    val |= 0x2
                if State.slot_status[i]['status2'] == 'protected':
                    val |= 0x8
                if State.slot_status[i]['status2'] == 'unprotected':
                    val &= 0x7
            elif State.slot_status[i]['status1'] in ['closed', 'mounted'] :
                val = 0x2
            else:
                val = 0

            # --- SHIFT THE DATA FOR THE SLOT
            sval = val << 4*(2 + i)
            rv |= sval

    return rv

###########################################################################
''' update_sg(slot) writes the meta data of eMSN and disk serial numbers to all disks
                    within the module and returns 0 for success, 1 for failure
                    this is called only after mod_init successfully completes. 
'''
def update_sg(islot_usg):
    # --- FOR READ-ABILITY
    sl_info = State.slot_info[islot_usg]
    log.debug("module_libs : sl_info is %s " % (sl_info))
    sl_status = State.slot_status[islot_usg]
    log.debug("module_libs : sl_status is %s " % (sl_status))

    disk_nm = 0    #--- COUNTER FOR DISK NOT MOUNTED
    # --- FIND THE STARTING DEVICE NUMBER
    # --- DEV_NUM IS SINCE SLOT INFO IS 1 AND 3 HAVE 0-7, 2 AND 4 8-15 
    dev_num = Utils.si_start(islot_usg)

    log.debug("module_libs : starting device number is %d" % (dev_num))
    dev_max = dev_num + sl_status['disks']
    log.debug("module_libs : ending device number is %d" % (dev_max))

    base_mnt_pt = State.mnt_meta + str(islot_usg) +'/'
    #print "base_mnt_pt = ", base_mnt_pt

    # --- THE SLOT CORRESPONDS FIRST MOUNT NUMBER
    mnt_pt = 0
    #print "mnt_pt = ", mnt_pt
    
    sn_list = []
    # --- J IS REQUIRED FOR 0-7, WHILE I CAN BE 0-15
    j = 0
    for i in range(dev_num,  dev_max):
        sn = str(j) + ":" + sl_info[i]['serial_num'] + "\n"
        #print "sn is ", sn
        sn_list.append(sn)
        j +=1

    log.info("module_libs : disk serial number list is %s " % sn_list)

    for i in range(dev_num,  dev_max):
        # --- VERIFY THE DEVICE META IS MOUNTED
        if sl_info[i].has_key('mmdev'):
            disk_dev = sl_info[i]['mmdev']
        else:
            log.info("module_libs : key mmdev did not exist for slot %d disk %d assigning mount point" % (islot_usg, i))
            # --- if a mmdev does not exist, look at first partition
            tmp_dev = sl_info[i]['dev']
            # --- remove it and add '2' for sg partition
            disk_dev = tmp_dev[:tmp_dev.rfind('1')] + '2'
            # --- reassign to slot_info 
            State.slot_info[islot_usg][i]['mmdev'] = disk_dev

        # --- THE FOLLOWING LINES COMMENTED OUT REPLACED WITH ROUTINE IS_MOUNTED

        #log.debug("module_libs : looking to see if %s is mounted " % (disk_dev))
        #p1 = subprocess.Popen(["df"], stdout=subprocess.PIPE)
        #p2 = subprocess.Popen(["grep", disk_dev], stdin=p1.stdout, stdout=subprocess.PIPE)
        #p1.stdout.close()  # Allow p1 to receive a SIGPIPE if p2 exits.
        #output = p2.communicate()[0]
        #out = mdf = output.split(" ")
        #print "mdf = ", mdf

        # --- IF THE DEVICE IS MOUNTED
        #if len(out) > 0 and out[0] == disk_dev:
        if is_mounted(disk_dev):
            log.debug("module_libs : device found %s " % (disk_dev))
        
            # --- IF IT IS, WRITE THE EMSN 
            wr_msn_f = base_mnt_pt + str(mnt_pt) + "/eMSN"
            eMSN_f = open(wr_msn_f, 'w')
            log.debug("module_libs : eMSN to write is %s " % sl_status['eMSN'])
            eMSN_f.write(sl_status['eMSN'])
            eMSN_f.close()
            log.info("module_libs : wrote %s to %s " % ( sl_status['eMSN'], wr_msn_f))

            # --- IF IT IS, WRITE THE DISK SERIAL NUMBERS  
            wr_sn_f = base_mnt_pt + str(mnt_pt) + "/disk_sn"
            sn_file = open(wr_sn_f, 'w')
            for j in range(sl_status['disks']):
                #log.debug("module_libs : disk %d writing %s " % (j,sn_list[j]))
                sn_file.write(sn_list[j])
            sn_file.close()
            log.info("module_libs : wrote slot : disk serial numbers %s  " % sn_list)

        else:
            log.debug("module_libs : device not mounted %s " % disk_dev)
            disk_nm += 1
        # --- INCREMENT TO NEXT MOUNT POINT
        mnt_pt += 1

    if disk_nm == 0:
        return 0
    else:
        return 1
###########################################################################
''' update_raid(slot) writes the meta data of eMSN and disk serial numbers to the RAID
                     module and returns 0 for success, 1 for failure
                    this is called only after group=new for a type raid mounts the disks
'''
def update_raid(lslot_rd):
    # --- since we can only make a raid out of a single module, read first param
    islot_rd = int(lslot_rd[0])

    # --- FOR READ-ABILITY
    sl_info = State.slot_info[islot_rd]
    log.debug("module_libs : sl_info is %s " % (sl_info))
    sl_status = State.slot_status[islot_rd]
    log.debug("module_libs : sl_status is %s " % (sl_status))

    disk_nm = 0    #--- COUNTER FOR DISK NOT MOUNTED
    # --- FIND THE STARTING DEVICE NUMBER
    # --- DEV_NUM IS SINCE SLOT INFO IS 1 AND 3 HAVE 0-7, 2 AND 4 8-15 
    dev_num = Utils.si_start(islot_rd)

    log.debug("module_libs : starting device number is %d" % (dev_num))
    dev_max = dev_num + sl_status['disks']
    log.debug("module_libs : ending device number is %d" % (dev_max))

    # --- DISK SERIAL NUMBER LIST
    sn_list = []
    # --- J IS REQUIRED FOR 0-7, WHILE I CAN BE 0-15
    j = 0
    for i in range(dev_num,  dev_max):
        sn = str(j) + ":" + sl_info[i]['serial_num'] + "\n"
        #print "sn is ", sn
        sn_list.append(sn)
        j +=1

    log.info("module_libs : disk serial number list is %s " % sn_list)

    # --- THIS ROUTINE IS ONLY CALLED WHEN A MODULE GROUP=NEW AND THE RAID IS MOUNTED.
    # --- IF IT IS, WRITE THE EMSN 
    wr_msn_f = State.raid_meta + "/"+ "eMSN"
    eMSN_f = open(wr_msn_f, 'w')
    log.debug("module_libs : eMSN to write is %s " % sl_status['eMSN'])
    eMSN_f.write(sl_status['eMSN'])
    eMSN_f.close()
    log.debug("module_libs : wrote eMSN to %s " % wr_msn_f)

    # --- IF IT IS, WRITE THE DISK SERIAL NUMBERS  
    wr_sn_f =  State.raid_meta +"/"+  "disk_sn"
    sn_file = open(wr_sn_f, 'w')
    for j in range(sl_status['disks']):
        log.debug("module_libs : disk %d writing %s " % (j,sn_list[j]))
        sn_file.write(sn_list[j])
    sn_file.close()
    log.debug("module_libs : wrote slot : disk serial numbers to %s " % wr_sn_f)

    return 0

######################################################################
''' get_group determines for the group reference passed if any module in the group 
    belongs to a pre-existing created group.  we will only check the meta 
    data on disk 0 for each module.

    it returns a list of the eMSNs in the group,
'''
def get_group(gg_lslot):
    rl = []   #--- EMPTY RETURN LIST
    gg_rv = 1
    # --- FOR EACH SLOT IN GROUP:
    for i in gg_lslot:
        ii = int(i)
        dev_check = Utils.si_start(ii)

        if State.slot_info[ii][dev_check].has_key('mmdev'):
            mount_dev = State.slot_info[ii][dev_check]['mmdev']
        
            log.debug("module_lib : evalutate group %s " % i)
            log.debug("module_lib : dev_check  %d " % dev_check)
            log.debug("module_lib : mount_dev %s " % mount_dev)
            # --- IS THE SLOT DISK 0 META DATA PARTITION MOUNTED
        
            if not is_mounted(mount_dev):
                log.debug("module_lib : %s is NOT mounted, mounting  " % mount_dev)
                # --- IF IT IS NOT, MOUNT IT TO THE APPROPRIATE MOUNT POINT
                gg_rv = mount_module(mount_dev, i, '0', 0)
                log.debug("module_lib : %s is mounted -> %d  " % (mount_dev,gg_rv))
            else: log.debug("module_lib : %s is mounted  " % mount_dev)

            #print "gg_rv ", gg_rv
            if gg_rv:
            
                # --- TEST IF THE FILE GROUP EXISTS
                g_dest = State.mnt_meta + i +'/' + '0' +'/' + "group"
                log.debug("module_libs : checking %s " % g_dest)
                if os.path.isfile(g_dest):
                    gf = open(g_dest, 'r')
                    rl.append(gf.readline())
                    gf.close()
                    log.debug("module_libs : update the rl to %s" % (rl))

    log.info("module_lib : returning contents of the group %s " % rl)
    return rl

######################################################################
''' verify_modules expects a list of the group reference that is trying to be opened
    and verifies that the modules of an existing created group are all present and
    in the proper slot.  

    It returns a 0 if it fails to verify and the list of the missing MSN's
'''
def verify_modules(lparam):

    for i in lparam:
        #print "vm - i ", i
        group_ref = ''
        # --- since get_group expects a list, create if for slot and get its group_info
        tmp = [i]
        # --- CHECK TO SEE IF INFORMATION EXIST ABOUT THE SLOT
        if slot_info[i].keys() :
            if slot_status[i]['type'] == 'sg':
            
                #print "calling get group with %s" % (tmp)
                gg_rv = module_libs.get_group(tmp)
                #print "get_group returned %s ", gg_rv
        
                if len(gg_rv) > 0:
                    gg_rv_l = gg_rv[0].split(":")
                    #  print "the number of modules in this group are %s" % (gg_rv_l[0])

                    for j in range(1, 5):
                        if slot_status[j]['eMSN'] in gg_rv[0]:
                            #print "the modules eMSN in slot %d is %s and it is a member of %s " % (j,slot_status[j]['eMSN'],gg_rv[0])
                            group_ref += str(j)
                    # --- IF AFTER CHECKING, THE GROUP_REF IS THE SAM
                    if int(gg_rv_l[0]) == len(group_ref):
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

                # --- else if raid device
                elif slot_status[i]['type'] == 'raid':
                    log.debug("module_lib : it is a raid dev, a group of one ")
                    

    return 1

###############################################################3
''' this routine mounts a disk or all disks in a slot and returns either 
    success or failure.  The failure, which disk failed to mount '''

def mount_module(dev, mm_slot, disk, start):
    num_disks = 1     #--- ASSUME ON DISK TO MOUNT
    immslot = int(mm_slot) # --- CONVERT THE SLOT NUMBER TO INT
    rv = 1
    sr = 0     #--- USED IN CASE DISK == ALL

    if disk == 'all':
        log.debug("module_libs : mount all disks for module in slot %s" % mm_slot)
        num_disks = State.slot_status[immslot]['rdisks']
        sr = start

    for i in range(num_disks):
        if disk == 'all':
            if '2' in dev:
                dev = State.slot_info[int(mm_slot)][sr]['mmdev']                
            else: 
                dev = State.slot_info[int(mm_slot)][sr]['dev']

            log.debug("module_libs : mount disk dev %s" % dev)

        # --- HERE, THE META DATA ARE NOT MOUNTED FOR ALL:  think
        # --- IF THE PARTITION CONTAINS A 1, IT IS A DATA PARTITION
        if '1' in dev:
            loc = State.mnt_data + mm_slot +'/'+ str(i)
            log.debug("module_libs : mount location is %s" % loc)
        elif '2' in dev:
            loc = State.mnt_meta + mm_slot +'/'+ str(i)
            log.debug("module_libs : mount location is %s" % loc)

        if not is_mounted(dev):
            log.debug("module_libs : device to mount is %s at %s" % (dev, loc))
            #res = subprocess.Popen(["sudo", "/bin/mount", "-t", "xfs", "-o", 'grpid', dev, loc],  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            #out, err = res.communicate()
            #line = err.split("\n")
            #if len(line) < 1:
            #    log.debug("module_libs : device %s is now mounted " % dev)
            #else: 
            #    if "error" in line[0]:
            #        log.error("module_libs : device %s did not mount " % dev)
            #        rv = 0

            cmd_to_ex = ["sudo", "/bin/mount", "-t", "xfs", dev, loc]
            log.debug("module_lib : send to execute %s " % cmd_to_ex)
            (rv, msg) = Utils.linux_cmd(cmd_to_ex)
            if not rv:
                State.add_error_msg(msg)
                rv &= 0

            cmd_to_ex = ["sudo", "chmod", "775", loc]
            log.debug("module_lib : send to execute %s " % cmd_to_ex)
            (rv, msg) = Utils.linux_cmd(cmd_to_ex)
            if not rv:
                State.add_error_msg(msg)
                rv &= 0

            cmd_to_ex = ["sudo", "chown", "root.mark6", loc]
            log.debug("module_lib : send to execute %s " % cmd_to_ex)
            (rv, msg) = Utils.linux_cmd(cmd_to_ex)
            if not rv:
                State.add_error_msg(msg)
                rv &= 0
            print "rv is ", rv
            print "dev is ", dev
            # --- IF THERE IS NOT AN ERROR
            if rv and '1' in dev:
                data_loc = loc + "/data"
                print "data_loc is ", data_loc
                if not os.path.isdir(data_loc):
                    log.info("updating data area on disk to %s" % (data_loc))
                    os.mkdir(data_loc)
                else:
                    log.debug(" data area on disk %s exists" % (data_loc))
                

        # --- INCREMENT START DEV LOCATION, IF DISK IS ALL
        sr += 1

    return rv

##############################################################################
# routine checks if a device is mounted returns, 1 if it is mounted, 0 if it is not    
def is_mounted(mount_dev):
    
    log.debug("module_libs : looking to see if %s is mounted " % (mount_dev))
    p1 = subprocess.Popen(["df"], stdout=subprocess.PIPE)
    p2 = subprocess.Popen(["grep", mount_dev], stdin=p1.stdout, stdout=subprocess.PIPE,stderr=subprocess.PIPE  )
    p1.stdout.close()  # Allow p1 to receive a SIGPIPE if p2 exits.
    output, err = p2.communicate()
    out = mdf = output.split(" ")
    #print "mdf = ", mdf
    #print "is mounted ", err
    
    # --- IF THE DEVICE IS MOUNTED
    if len(out) > 0 and out[0] == mount_dev:
        log.debug("module_libs : device found %s " % (out[0]))
        return 1
    else:
        log.debug("module_libs : device %s was not found : error %s" % (mount_dev, err))
        return 0

##############################################################################
# Assumes the modules being grouped are not in a group, the 
#  routine creates group meta data and writes it to all disks in group    
def create_group(cg_lslot):

    log.debug("module_libs - Entered with %s" % cg_lslot)
    rl = []   #--- EMPTY RETURN LIST
    cg_rv = 1
    num_members = len(cg_lslot)
    #print "num_members ", num_members
    # --- INITIALIZE THE GROUP

    group = str(num_members) + ":"
    log.debug("module_libs : group is %s " % group)
    # --- FOR EACH SLOT IN GROUP:
    for i in cg_lslot:
        ii = int(i)
        group += State.slot_status[ii]['eMSN'] + ":"
        
    # --- REMOVE TRAILING :
    group = group.rstrip(":")
    log.info("module_libs : group meta data to be written is %s " % group)

    for i in cg_lslot:
        ii = int(i)
        dev_start = Utils.si_start(ii)

        # --- mount all the meta drives for the group, 
        if mount_module('2', i, 'all', dev_start):
            # --- if all mounts were successful, then update the group file
            # --- should we change permissions here first?
            for k in range(8):

                cg_dest = State.mnt_meta + i +"/" + str(k) + "/" + "group" 

                log.debug("module_libs : writing %s " % cg_dest)
                gf = open(cg_dest, 'w')
                gf.write(group)
                gf.close()

    if cg_rv:  log.info("module_lib : wrote group to all members, returning %d " %cg_rv)
    else:  log.warning("module_lib : all members of the group were not updated %d " %cg_rv)

    return cg_rv

##############################################################################
# Assumes the modules being grouped are not in a group, the 
#  routine creates group meta data and writes it to a raid0 disk
def create_group_raid(cg_lslot):

    log.debug("module_libs - Entered with %s" % cg_lslot)
    rl = []   #--- EMPTY RETURN LIST
    cg_rv = 1
    num_members = len(cg_lslot)
    #print "num_members ", num_members
    # --- INITIALIZE THE GROUP

    group = str(num_members) + ":"
    log.debug("module_libs : group is %s " % group)
    # --- FOR EACH SLOT IN GROUP:
    for i in cg_lslot:
        ii = int(i)
        group += State.slot_status[ii]['eMSN'] + ":"
        
    # --- REMOVE TRAILING :
    group = group.rstrip(":")
    log.info("module_libs : group meta data to be written is %s " % group)

    for i in cg_lslot:
        ii = int(i)

        cg_dest = State.raid_meta +"/"+  "group" 

        log.debug("module_libs : writing %s " % cg_dest)
        gf = open(cg_dest, 'w')
        gf.write(group)
        gf.close()

    if cg_rv:  log.info("module_lib : wrote group to all members, returning %d " %cg_rv)
    else:  log.warning("module_lib : all members of the group were not updated %d " %cg_rv)

    return cg_rv
    
###################################################################
def sg_unmount(sgd_slot):
    if type(sgd_slot) != 'str':
        sgd_slot = str(sgd_slot)

    if type(sgd_slot) != 'int':
        isgd_slot = int(sgd_slot)
    else: isgd_slot = sgd_slot

    rv = 1

    for i in range(State.slot_status[isgd_slot]['rdisks']):
        mnt_pt = State.mnt_data+sgd_slot+'/'+str(i)
        log.debug("module-lib : unmounting %s " % mnt_pt)
        # --- UNMOUNT THE DISK
        res = subprocess.Popen(["sudo", "umount", mnt_pt],  stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        out, err = res.communicate()
        line = err.split("\n")
        if line == '':
            log.debug("module_libs : device %s is now unmounted " % mnt_pt)
        else: 
            if "error" in line[0]:
                log.error("module_libs : devices %s did not unmount " % mnt_pt)
                rv = 0

        mnt_meta_pt = State.mnt_meta+sgd_slot+'/'+str(i)
        log.debug("module-lib : unmounting %s " % mnt_meta_pt)

        res = subprocess.Popen(["sudo", "umount", mnt_meta_pt],  stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        out, err = res.communicate()
        line = err.split("\n")
        if len(line) < 1:
            log.debug("module_libs : meta device %s is now unmounted " % mnt_meta_pt)
        else: 
            if "error" in line[0]:
                log.error("module_libs : meta device %s did not unmount " % mnt_meta_pt)
                rv = 0

    # --- MAaY NEED TO CHANGE THE STATUS OF THE MNT POINT IN SLOT_INFO HERE

    return rv
    
###################################################################
def remove_group(rg_slot, diskfmt):

    if type(rg_slot) != 'str':
        rg_slot = str(rg_slot)

    if type(rg_slot) != 'int':
        irg_slot = int(rg_slot)
    else: irg_slot = rg_slot

    rv = 1

    if diskfmt == 'sg':
        dev_start = Utils.si_start(rg_slot)
        trv = mount_module('2', rg_slot, 'all', dev_start)
        #print "trv is ", trv
        # --- remove group file from meta dir
        for i in range(State.slot_status[irg_slot]['rdisks']):
            group_f = State.mnt_meta+rg_slot+'/'+str(i)+'/'+'group'
            if(os.path.isfile(group_f)):
                cmd_to_ex = ["rm", group_f]
                (rv, msg) = Utils.linux_cmd(cmd_to_ex)
                if not rv:
                    State.add_error_msg(msg)
                    rv &= 0
                    log.warning("module_libs : Could not remove slot %d disk %d from group " % (irg_slot, i))

                else:
                    log.debug("module_libs : removed slot %d disk %d from group " % (irg_slot, i))
        
        if rv:
            log.info("module_libs : Success in removing module in slot %d from group " % (irg_slot))


    return rv

######################################################################
''' slot mounted looks to see if for the diskfmt, that all the disks are mounted.
    if they are, it returns 1, if not, 0
'''
def slot_mounted(s_slot, diskfmt):
    
    if type(s_slot) != 'str':
        s_slot = str(s_slot)

    if type(s_slot) != 'int':
        is_slot = int(s_slot)
    else: is_slot = s_slot

    rv = 1

    if diskfmt == 'sg':
        dev_check = Utils.si_start(is_slot)
        dev_end = dev_check + 8
        for i in range(dev_check, dev_end):

            # --- CHECK DATA PARTITION,
            if State.slot_info[is_slot][dev_check].has_key('dev'):
                mount_dev = State.slot_info[is_slot][dev_check]['dev']
        
                log.debug("module_lib : evalutate group %s " % is_slot)
                log.debug("module_lib : dev_check  %d " % dev_check)
                log.debug("module_lib : mount_dev %s " % mount_dev)
                
                if is_mounted(mount_dev):
                    if State.slot_info[is_slot][dev_check].has_key('mmdev'):
                        mount_dev = State.slot_info[is_slot][dev_check]['mmdev']

                        if not is_mounted(mount_dev):
                            log.debug("module_lib : fail %s is not mounted " % mount_dev)
                            rv = 0
                            break
                else:
                    log.debug("module_lib : fail %s is not mounted " % mount_dev)
                    rv = 0
                    break

    else:
        md_slot = find_mdslot(is_slot)
        if not is_mounted(md_slot): 
            log.debug("module_lib : fail %s is not mounted " % mount_dev)
            
            rv = 0

    return rv

########################################################################
# This routine removes sn from the disk module.  It is passed the group_ref (gref)
# the scan name (sn) and the disk format type (sg, raid).  It will erase the scan.
#
def delete_scan(gref, sn, dt):
    
    log.info("module_libs : deleting %s from group %s" % (sn, gref))
    log.debug("module_libs : delete %s from gref %s, type %s" %(gref, sn, dt))
    lfiles = []
    if dt == 'sg':
        sgref = "["+gref+"]"
        loc = State.mnt_data+sgref+"/[0-7]"
        sn_ext = "/"+sn+".vdif"
        log.debug("module_libs : remove %s" % loc)
        temp_lfiles = glob(loc)    
        lfiles = [x + sn_ext for x in temp_lfiles]
    # --- ELSE IT IS A RAID MODULE
    else:
        loc = State.raid_data+"/"+sn+".vdif"
        lfiles.append(loc)

    log.debug("module_libs : removing %s" % lfiles)
    if len(lfiles) > 0:
        for fn in lfiles:
            os.remove(fn)
            log.debug("module_libs : removed %s" % fn)
            #scan_name = fn[fn.rfind("/")+1:]
            scan_name = sn
            log.info("scan_name removed was %s" % scan_name )
            if State.fn_dict.has_key(scan_name):
                scan_num =  State.fn_dict[scan_name]
                #print "scan_num removed was ", scan_num
                del State.fn_dict[scan_name]
                #print "removed scan_name from dict ", scan_name
                del State.file_dict[scan_num]
                log.debug("removed scan_num %d from dict " % scan_num)
            else:
                log.warning("did not have key for %s" % State.fn_dict)
        return(0)
    else:
        msg = "scan %s for group %s could not be found" % (sn, gref)
        State.add_error_msg(msg)
        return(1)
########################################################################
# This routine removes sn from the disk module.  It is passed the group_ref (gref)
# the scan name (sn) and the disk format type (sg, raid).  It will erase the scan.
#
def delete_all_scans(gref, dt):
    global file_dict

    print "in delete all scans"
    log.debug("module_libs : delete all scans from gref %s, type %s" %(gref, dt))
    lfiles = []
    ret = 0   # SUCCESS
    if dt == 'sg':
        sgref = "["+gref+"]"
        loc = State.mnt_data+sgref+"/[0-7]/data"
        print "loc ", loc
        loc_slist = State.mnt_meta+sgref+"/[0-7]/slist" 
        print "loc_slist ", loc_slist
        temp_data = glob(loc)  
        print "temp_lfiles ", temp_data
        #lfiles = temp_data
        temp_lfiles = glob(loc_slist)    
        print "temp_lfiles ", temp_lfiles
        lfiles = temp_lfiles

        print "lfiles = ",lfiles
        print "temp_data =", temp_data

    # --- ELSE IT IS A RAID MODULE
    else:
        loc = State.raid_data + "/data"
        loc_slist = State.raid_meta+'/slist'
        temp_data=glob(loc)
        temp_lfiles = glob(loc_slist)    
        lfiles = loc_slist

    log.debug("module_libs : removing %s" % lfiles)
    if len(lfiles) > 0:
        # --- FOR EACH MOUNTED DATA POINT
        for d in temp_data:
            # --- MOVE DATA DIRECTORY TO TMP
            temp_dir = d[:d.rfind("/")+1] + "tmp"
            shutil.move(d, temp_dir)
            print "moving %s to %s " % (d, temp_dir)

            # --- CREATE THE DATA DIRECTORY AGAIN FOR FUTURE USE.
            os.mkdir(d)
            print "created %s " % (d)

            #os.chmod(temp_dir,)
            # --- REMOVE THE UNLINK THE DIRECTORY TMP
            shutil.rmtree(temp_dir)
            print "unlinked %s " % (temp_dir)


        #
        for fn in lfiles:
            print "remove slist on ", fn
            os.remove(fn)

        # --- RESET THE FILE_DICT
        file_dict = {}        

        return(ret)
        
    else:
        log.warning("no scans exist for group %s, module is empty" % (gref))
        return(1)

##########################################################3
# CREATES THE LISTING OF A NEW GROUP THAT IS OPENED   
#    
def get_dir_listing(gref,directory):
    sfl = []
    file_dict = {}
    mtime = lambda f: os.stat(os.path.join(directory, f)).st_mtime
    sfl = list(sorted(os.listdir(directory), key=mtime))
    #print "sfl ", sfl
    sn = 1
    for i in sfl:
        name = i[:i.rfind(".")]
        log.debug("Processing %s " % i)
        a = os.stat(os.path.join(directory,i))
        size = scan_size(gref, i)
        # --- MAYBE MAKE THIS AN KEY ENTRY, E.G. SCAN NUMBER
        # --- SINCE A LISTING OF A CLOSED MODULE IS POSSIBLY, THIS SHOULD NOT BE A STATE.
        #     FOR ONE CAN HAVE AN OPEN GROUP AND READ A CLOSED MODULE FROM A NOTHER GROUP.
        #State.file_dict[sn] = {'sn':name, 'status':'recorded', 'start_tm':'unk', 'dur':'unk', 'num_str':'unk', 'spc':'unk', 'create_time':time.ctime(a.st_ctime), 'size':size} #[sn,created,size]
        file_dict[sn] = {'sn':name, 'status':'recorded', 'start_tm':'unk', 'dur':'unk', 'num_str':'unk', 'spc':'unk', 'create_time':time.ctime(a.st_ctime), 'size':size} #[sn,created,size]
        # --- CREATE THE HASH TABLE FOR FILE NAME LOOKUP
        #State.fn_dict[name] = sn
        #print file_dict[sn]
        sn += 1
    #return State.file_dict
    return file_dict

###########################################################
# AFTER A SCAN IS RECORDED, UPDATES THE LISTING DICTIONARY
def add_file_to_listing(sl, status, start, duration, num_streams, spc):
    
    # --- INCREMENT THE SCAN NUMBER
    adtl_sn = State.sn
    adtl_sn += 1
    if not State.file_dict.has_key(adtl_sn):
        State.sn = adtl_sn
        # --- start time may be a time_struct, if it is convert to secs from epoct time for consistency
        if type(start) == time.struct_time:
            start = timegm(start)

        # --- SET THE CREATION_TIME TO NOW
        create_tm  = time.strftime('%Yy%jd%Hh%Mm%Ss',time.gmtime())

        # --- MAYBE MAKE THIS AN KEY ENTRY, E.G. SCAN NUMBER
        State.file_dict[State.sn] = {'sn':sl, 'status':'pending', 'start_tm':start, 'dur':duration, 'num_str':num_streams, 'spc':spc, 'create_time':create_tm, 'size':0} #[sn,created,size]
        # --- THIS ASSUMES THAT WHEN A RECORD WAS ISSUED, IT CHECKED TO SEE IF THE FILENAME EXISTS AND SLIGHTLY CHANGED IT
        log.debug("adding %s, as scan_number %d \n" % (sl, State.sn))
        State.fn_dict[sl] = State.sn
        return 1
    else:
        log.info("adding %s failed, since scan_number %d was already in dictionary \n" % (sl, adtl_sn))
        return 'NULL'

###########################################################
# AFTER A SCAN IS RECORDED, UPDATES THE LISTING DICTIONARY
def commit_file_listing(gref, ufl_sn, status ):
    State.file_dict
    # --- IF THE FILE EXISTS?
    if os.path.isfile(fn_full):
        # --- GET THE FILE POSIX.STAT_RESULTS
        fn_stat = os.stat(fn_full)
        # --- EXTRACT ONLY THE FILENAME FROM FILENAME WITH FULL PATH
        fn = fn_full[fn_full.rfind("/")+1:]

        size = scan_size(gref, fn)
        # --- MAYBE MAKE THIS AN KEY ENTRY, E.G. SCAN NUMBER
        State.file_dict[State.sn]['create_time']=time.ctime(fn_stat.st_ctime)
        State.file_dict[State.sn]['size'] = size #[sn,created,size]
    
        return fn
    else:
        return 'NULL'

#############################################################3
# DETERMINES THE SIZE OF THE SCAN THAT WAS RECORDED AND RETURNS SIZE
def scan_size(gref, fn):
    # --- FOR GREF, create a list
    gl = list(gref)
    fs = 0

    if ".vdif" not in fn:
        fn = fn + ".vdif"

    log.debug("Calculate file size for %s" % fn)

    # --- IF IT IS NOT ZERO LENGTH
    if len(gl) != 0:
        # --- FOR EACH MEMBER OF GROUP
        for i in gl:
            ii = int(i)
            # --- NUMBER OF DEVICES TO EXPECT
            devs = State.slot_status[ii]['rdisks']
            for j in range (devs):
                #print "i type ", type(i)
                #print "j = ", j
                #print "type(j) ", type(j)
                #file_loc = State.mnt_data + i + '/'+ str(j) +'/'+fn
                file_loc = State.mnt_data + i + '/'+ str(j) +'/data/'+fn
                #print "floc ", file_loc
                file_info = os.stat(file_loc)
                #print "finfo ", file_info
                fs += file_info.st_size
                #print "fs ", fs
        # --- CONVERT TO GBYTES
        fs = "%.3f" % (fs/1000000000.)
        #print "final fs ", fs

    log.debug("File size is %s" % fs)

    return fs

####################################################################3
# write groups listing to meta data location, action {new or append}
# and sn (the scan number you would like to write)
def write_list(gref, action, sn):
    # for all disks in the gref
    #print "action ", action 
    #print "type ", type(action)
    log.debug("write scan info to slist %s" % action)
    gl = list(gref)
    # --- IF IT IS NOT ZERO LENGTH
    if len(gl) != 0:
        
        if sn != 0:
            if State.file_dict[sn]['size'] == 0:
                State.file_dict[sn]['size'] =  scan_size(gref, State.file_dict[sn]['sn'])
                #print type(State.file_dict[sn]['size'] )
                log.debug("updating scan size to : %s" % State.file_dict[sn]['size'])
        
        if action == 'new':

            # --- FOR EACH MEMBER OF GROUP
            for i in gl:
                ii = int(i)
                # --- NUMBER OF DEVICES TO EXPECT
                devs = State.slot_status[ii]['rdisks'] 
                #print "write_list, devs - ", devs
                #print "write_list, State.file_dict ", State.file_dict
                for j in range (devs):
                    fn = State.mnt_meta + i + "/"+str(j) +"/slist"
                    #print "write_list, fn ", fn
                                        
                    target = open(fn, 'w')
                    target.write(str(State.file_dict))
                    target.close()
                    #print "write_list wrote fn"

            return 1

        elif action == 'append':
            # --- FOR EACH MEMBER OF GROUP
            # --- CHECK IF THE SIZE OF THE SCAN IS ZERO, IF IT IS, CHECK IF IT WAS NOT UPDATED
            
            for i in gl:
                ii = int(i)
                # --- NUMBER OF DEVICES TO EXPECT
                devs = State.slot_status[ii]['rdisks'] 

                for j in range (devs):
                    fn = State.mnt_meta + i + "/"+str(j) +"/slist"
                    target = open(fn, 'w')
                    target.write(str(State.file_dict))
                    target.close()
            return 1

        else:
            return 0

    else:
        return 0

####################################################################3
# read groups listing from meta data location
def read_list(gref):
    file_dict = {}
    
    # for all disks in the gref
    gl = list(gref)
    #print "gl = ", gl
    # --- IF IT IS NOT ZERO LENGTH
    if len(gl) != 0:  
        fn = State.mnt_meta + gl[0] + "/"+str(1) +"/slist"
        log.debug("does %s exist " % fn)
        if os.path.isfile(fn): 
            log.debug("yes, open and read scan_list ")            
            #fn_list = open(fn, 'r').read()
            fn_list = open(fn, 'r')
            #print "file opened for read"
            data = fn_list.read()
            #print "data ", data
            #State.file_dict = ast.literal_evan(fn_list)
            file_dict = ast.literal_eval(data)
            #print "file_dict set ", file_dict
            fn_list.close()
            #print "close file"

        else:
            # --- CHECK THE FIRST SLOT OF GREF TYPE AND ASSIGN DESTINATION LOCATION
            if State.slot_status[int(gl[0])]['type'] == 'sg':
                #dloc = State.mnt_data+ "/" + gl[0] + "/0/"
                dloc = State.mnt_data + gl[0] + "/0/data/"
            elif State.slot_status[int(gl[0])]['type'] == 'raid':
                dloc = State.mnt_raid + '/data/'
            else:
                # --- A UNKNOWN TYPE, THEREFORE, SET TO -
                dloc = "-"

            # --- IF SLOT TYPE IS KNOWN GET THE DIRECTORY LISTING
            if dloc != "-":
                log.debug("no file, create scan_list ")            
                file_dict = get_dir_listing(gref, dloc)
                
    #print "leaving read_list with ", file_dict
    return file_dict


