## Copyright 2011 MIT Haystack Observatory
##
## This file is part of Mark6 c-plane.  It contains utility functions
## used by some commands.
##
## Mark6 cplane is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 2 of the License.
##
## Mark6 cplane is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Mark6 cplane.  If not, see <http://www.gnu.org/licenses/>.
'''
Created on Oct 3, 2011

@author: cruszczyk

Utils.py is a number of useful utilities for communication with dplane and time conversions.

'''

import sys
import os
import logging
import time
import socket
import State
import binascii
import subprocess
import string
import Parser
import module_libs

log = logging.getLogger("M6")

dplane_comm_state = 0  # --- INITIALIZE STATE OF COMMUNICATION CONNECTION TO D-PLANE AS DISCONNECTED.

def get_pypath_resource(resource_name):
    for path in sys.path:
        if os.path.exists(os.path.join(path, resource_name)):
            return os.path.join(path, resource_name)
    raise Exception('Resource %s could not be found' % resource_name)
 
def set_log_level(level):
    global log

    print "entered set_log_level with level ", level
    LOG_LEVEL_MAP = {
        '0':  logging.DEBUG,
        '1':  logging.INFO,
        '2':  logging.WARNING,
        '3':  logging.ERROR,
        '4':  logging.CRITICAL
        }
    try:
        level = LOG_LEVEL_MAP.get(level, logging.INFO)
    except:
        level = logging.INFO

    print "log level is set to %s " % level
    #logging.basicConfig(level=level)
    
    stamp = time.strftime('%Y.%j.%H.%M.%S',time.gmtime());
    logfile = 'M6-' + stamp + '.log'
    print "logfile is %s" % logfile

    log.setLevel(level)
    # create file handler which logs even debug messages
    fh = logging.FileHandler(logfile)

    # --- original for debug purposes
    #fh.setLevel(logging.DEBUG)
    # --- SET IT TO WHAT IS PASSED
    fh.setLevel(level)

    # create console handler with a higher log level
    ch = logging.StreamHandler()
    ch.setLevel(logging.WARNING)
    # create formatter and add it to the handlers
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(funcName)s - %(levelname)s - %(message)s')
    ch.setFormatter(formatter)
    fh.setFormatter(formatter)
    # add the handlers to logger
    log.addHandler(ch)
    log.addHandler(fh)

    #print "exit set_log_level"


def vextime_to_standard(vtime):
    log.debug("vtime conversion %s" % vtime)
    # --- was year passed
    time_struct = time.gmtime()
    valid_val = 0
    vextime_error = 0

    y_loc = vtime.find("y")
    if y_loc < 0:
        # --- use present year
        year = str(time_struct.tm_year)
    else:
        # --- THE COMMAND EXPECTS THE FORMAT XXy SO ADDING 20 FOR 20XX AS YEAR
        year = str(vtime[:y_loc])
        iyear = int(year)
        #print "iyear is ", iyear
        if iyear >= 0 and iyear <100:
            valid_val=1
            year = "20"+year
            iyear = int(year)

        if iyear > 2000 and iyear < 2040:
            valid_val = 1
            log.info("Utils : 4 digit year was passed, using it %s " % year)
        else:
            log.error("vextime received invalid year entry %s" % year)
            vextime_error = 1

    d_loc = vtime.find("d")
    if d_loc < 0:
        # --- use present year
        doy = str(time_struct.tm_yday)
    else:
        if y_loc < 0:
            doy = vtime[:d_loc]
        else:
            doy = vtime[y_loc+1:d_loc]
            idoy = int(doy)
            if idoy >= 0 and idoy <=365:
                valid_val=1
            else:
                log.error("vextime received invalid DOY entry %s" % doy)            
                vextime_error = 1            

    h_loc = vtime.find("h")
    #print "h_loc is ", h_loc
    if h_loc < 0:
        # --- use present year
        hr = str(time_struct.tm_hour)
        #print "h_loc is < 0 hr is ", hr
    else:
        if d_loc < 0:
            hr = vtime[:h_loc]
        else:
            hr = vtime[d_loc+1:h_loc]        
            ihr = int(hr)
            #print "hour is : ", ihr
            if ihr >= 0 and ihr < 24:
                valid_val=1
            else:
                log.error("vextime received invalid hour entry %s" % hr)            
                vextime_error = 1

    m_loc = vtime.find("m")
    if m_loc < 0:
        # --- use present minute
        minute = str(time_struct.tm_min)
    else:
        if h_loc < 0:
            minute = vtime[:m_loc]
        else:
            minute = vtime[h_loc+1:m_loc]
            imin = int(minute)
            if imin >= 0 and imin < 60:
                valid_val=1
            else:
                log.error("vextime received invalid minute entry %s" % minute)            
                vextime_error = 1

    s_loc = vtime.find("s")
    if s_loc < 0:
        # --- use present sec
        sec = str(time_struct.tm_sec)
    else:
        if m_loc < 0:
            sec = vtime[:s_loc]
        else:
            sec = vtime[m_loc+1:s_loc]
            isec = int(sec)
            if isec >= 0 and isec < 60:
                valid_val=1
            else:
                log.error("vextime received invalid second entry %s" % sec)            
                vextime_error = 1
    
    # --- CONVERT TO THE TIME STRUCT AND RETURN 
    if vextime_error == 0 and valid_val == 1:

        str_time = year + " " + doy + " " + hr + " " + minute + " " + sec
        ret_time = time.strptime(str_time, "%Y %j %H %M %S")
        log.info("vextime return time is %s" % (time.asctime(ret_time)))

    else:
        ret_time = "-1"
        log.warning("vextime return time is %s" % ret_time)
        valid_val = 0

    return (valid_val, ret_time)
#################################
def vex2vdif(vex):
    log.debug("vex2vdif, time to convert is %s" % (vex))
    (res, times) = vextime_to_standard(vex)
    log.debug("vex2vdif : res = %d, times = %s" % (res, times))

    secs = 0
    minute = 0
    hour = 0
    mday = 1
    wday = 0
    yday = 0
    isdst = 0
    
    # --- if the vextime was in a valid format
    if res > 0:

        time_in_secs = time.mktime(times)
        log.info("vex2vdif: vex time in secs %d " % (time_in_secs))
        epoch = (times.tm_year-2000)*2
        log.debug("vex2vdif : epoch is %d" % (epoch))
        # --- IF YEAR IS YEAR SINCE 1900
        if epoch < 0:
            log.error("vex2vdif epoch is invalid return ")
            return(-1)
        
        log.debug("vex2vdif: valid epoch")
        # --- IF THE MONTH IS THE FIRST SIX
        #log.debug( "vex2vdif month is %d " % (times.tm_mon))
        
        month = times.tm_mon
        if month < 7:
            # --- SET THE MONTH TO 1
            month = 1
            log.info("vex2vdif : setting month to 1")
        # --- ELSE ITS THE SECOND HALF OF YEAR July
        else:
            month = 7
            epoch +=1
            log.debug("vex2vdif : epoch is now %d " % ( epoch))

        epoch %= 32
        ref_time = (times.tm_year,
                    month,
                    mday,
                    hour,
                    minute,
                    secs, 
                    wday,
                    yday,
                    isdst)

        base_time = time.mktime(ref_time)
        log.debug( "vex2vdif : the base_time is %d" % ( base_time))
        
        vdif_secs = time_in_secs - base_time
        log.info("vex2vdif: new_time.secs is %d "% (vdif_secs))
        log.info("vex2vdif: final epoch is %d"% (epoch))

        return(epoch, vdif_secs)

    else:
        log.error("vextime_to_standard returned error")
        return (-1, -1)


###############################
def d_plane_comm(host, cmd, cmd_struct):
    global log
    global dplane_comm_state
    global dplane

    host = "127.0.0.1"
    port = 7213

    log.debug(" Utils:d_plane_comm - state is %d - sending %s to localhost" % (dplane_comm_state, cmd))

    if dplane_comm_state == 0:

        # --- USING UDP SOCKETS. SO MUST CHANGE.
        #for res in socket.getaddrinfo(host, port, socket.AF_UNSPEC, socket.SOCK_STREAM):
        for res in socket.getaddrinfo(host, port, socket.AF_UNSPEC, socket.SOCK_DGRAM,):
            af, socktype, proto, canonname, sa = res
            try:
                dplane = socket.socket(af, socktype, proto)
            except socket.error, msg:
                dplane = None
                continue
            #try:
            #    dplane.connect(sa)
            #except socket.error, msg:
            #    dplane.close()
            #    dplane = None
            #    continue
            #break
    
        if dplane is None:
            log.error('Utils:d_plane_comm -could not open socket to d-plane at %s', host)
            return(0)
        else:
            log.debug('Utils:d_plane_comm - ddplane socket configured')
            dplane_comm_state = 1

    if cmd != "close":
        log.info("Utils:d_plane_comm -sending %s to %s", cmd.strip(), host)

        #print 'message being sent is', binascii.hexlify(cmd_struct)

        rv = dplane.sendto(cmd_struct, (host,port))

        if rv <= 0 :
            log.error("Utils:d_plane_comm -Failed to send %s to %s ", cmd.strip(), host)
            log.error("Utils:d_plane_comm - seq_no not incremented")
            return(1)
        else:
            log.info("Utils:d_plane_comm -Sent %s to d-plane at %s", cmd.strip(),host)
            State.seqno += 1
            log.debug("Utils:d_plane_comm - seq_no incremented to %d" % State.seqno)

            return(0)
    else:
        # --- close the socket
        log.info("Utils:d_plane_comm - Communication to d-plane closed")                 
        dplane.close()
        # --- reset dplane_comm_state
        dplane_comm_state = 0
        return(0)
#########################################
''' Get individual disk information on a specific partition, 
    e.g. /dev/sda1 
'''

def disk_info_p(dev):
    size = 0
    usage_flag = 0
    partition_num = "-"
    rv = {}
    mount_path = '-'
    size = '-'
    partition_num = '-'
    fs_type = '-'
    usage = '-'
    partition_label = '-'

    # --- USING UDISK GET ALL INFORMATION ON THE DEVICE
    res = subprocess.Popen(["udisks", "--show-info", dev],  stdout=subprocess.PIPE)
    #print("just called udisks ")
    out = res.communicate()[0]
    # --- THE OUTPUT IS SEPARATED BY CR"
    disk_info = out.split("\n")
    #print disk_info
    #log.debug("Utils.disks() : disk_info - %s, %d " % (disk_info, len(disk_info)))
    #log.debug("Util : len of disk_info is %d" % (len(disk_info)))



    for i in range(len(disk_info)):
        tmp_disk_info = disk_info[i].split()
        #print("Util : tmp_disk_info is %s" % (tmp_disk_info))
        if "  size:" in disk_info[i]:
            log.debug("Utils (%s) size found for disk " % (dev) )
            #log.debug("Utils : len of tmp_disk_info is %d" % len(tmp_disk_info))
            if len(tmp_disk_info) > 1:
                size = tmp_disk_info[1]

            log.debug("Utils (%s) size is %s" % (dev, size))
            #print "disk_info[i] = %s, size is %s" % (disk_info[i], size)

        elif "usage:" in disk_info[i]:
            if len(tmp_disk_info) > 1:
                usage = tmp_disk_info[1]
                #print "if usage = ", usage
            #print "usage =", usage
            if usage == 'filesystem':
                log.info("Utils (%s) setting usage to SG for disk " % (dev))
                usage = 'sg'

            usage_flag = 1

        elif "is mounted:" in disk_info[i]:
            mount_flag = 0
            #log.debug("Utils : is mounted disk_info %s  " % ( disk_info ))
            #print "Utils : is mounted tmp_disk_info -> ", tmp_disk_info
            if len(tmp_disk_info) > 1:
                if tmp_disk_info[2] == '1':
                    mount_flag = 1
                    log.debug("Utils (%s), disk mounted " % (dev))
                else:
                    log.debug("Utils (%s), disk is NOT mounted" % (dev))

        elif "mount paths:" in disk_info[i] and mount_flag:
            mount_path = tmp_disk_info[2]
            mount_flag = 0

        elif "partition:" in disk_info[i]:
            partition_flag = 1
            log.debug("Utils (%s), partitions exist " % (dev))

        elif "number:" in disk_info[i] and partition_flag:
            partition_num = tmp_disk_info[1]
            partition_flag = 0

        elif "  label:" in disk_info[i]:
            if  len(tmp_disk_info) > 1:
                partition_label = tmp_disk_info[1]
            #print "label is ", partition_label
            
        elif "type:" in disk_info[i] and usage_flag:
            if  len(tmp_disk_info) > 1:
                fs_type = tmp_disk_info[1]
            else: fs_type = 'unk'
            log.info("Utils (%s), File system type for disk is %s " %(dev, fs_type))
            usage_flag = 0

    rv = {'dev':dev, 'size':size, 'mount_pt':mount_path, 'fs':fs_type, 'part_num':partition_num, 'usage':usage, 'part_label':partition_label}

    return rv
#########################################
# Get individual disk information e.g. /dev/sda

def disk_info_np(dev):
    size = 0
    partition_num = "0"
    rv = {}
    temp = '-'
    model = '-'
    serial_num = "-"
    usage = "-"
    vendor = "-"
    temp = "-"
    size = "-"
    disk_type = "-"
    mount_path = "-"
    fs_type = 'unk'

    # --- USING UDISK GET ALL INFORMATION ON THE DEVICE
    res = subprocess.Popen(["udisks", "--show-info", dev],  stdout=subprocess.PIPE)
    #print("just called udisks ")
    out = res.communicate()[0]
    # --- THE OUTPUT IS SEPARATED BY CR"
    disk_info = out.split("\n")
    #print disk_info
    #log.debug("Utils.disks() : disk_info - %s, %d " % (disk_info, len(disk_info)))
    #log.debug("Util : len of disk_info is %d" % (len(disk_info)))
    for i in range(len(disk_info)):
        tmp_disk_info = disk_info[i].split()
        #print("Util : tmp_disk_info is %s" % (tmp_disk_info))
        if "  size:" in disk_info[i]:
            log.debug("Utils (%s) disk %d size found" % (dev, i) )
            #print("len of tmp_disk_info is %d" % len(tmp_disk_info))
            if len(tmp_disk_info) > 1:
                size = tmp_disk_info[1]

            log.info ("Utils (%s) disk %d size is %s" %(dev, i, size))
                
            #print "disk_info[i] = %s, size is %s" % (disk_info[i], size)
        elif "  serial:" in disk_info[i]:
            if len(tmp_disk_info) > 1:
                serial_num = tmp_disk_info[1]

            log.info("Utils (%s) disk %d serial_num is %s" % (dev, i, serial_num))

        elif "  model:" in disk_info[i]:
            model = "-"
            if len(tmp_disk_info) > 1:
                model = ""
                for i in range(1, len(tmp_disk_info)):
                    model = model + tmp_disk_info[i] + " "

            log.info("Utils (%s) disk %d model is %s" % (dev, i, model))

            #print "serial is ", serial_num
        elif "usage:" in disk_info[i]:
            if len(tmp_disk_info) > 1:
                usage = tmp_disk_info[1]

            log.info("Utils (%s) disk %d usage is %s " % (dev, i, usage))
            #print "usage is ", usage
        elif "vendor:" in disk_info[i]:
            if len(tmp_disk_info) > 1:
                vendor = tmp_disk_info[1]

            log.info("Utils (%s), disk %d vendor is %s " % (dev, i, vendor))
            #print "usage is ", usage

        elif "temperature-celsius-2" in disk_info[i]:
            #print "tdi ", tmp_disk_info
            if len(tmp_disk_info) > 1:
                temp = tmp_disk_info[4]
            log.info("Utils (%s), disk %d temperature is %s C " % (dev, i, temp))
            #print "usage is ", usage

        elif "rotational media:" in disk_info[i]:
            rotate = disk_info[i].split(":")
            #log.debug("Utils :  disk_info_np(%s), disk %d rotate is %s" % (dev, i, rotate))
            if len(rotate) > 1:
                if "No" in rotate[1]:
                    disk_type = "SSD"
                else:
                    disk_type = "Rotating"

            log.info("Utils (%s), disk %d disk type is %s" % (dev, i, disk_type))
            #print "md type is ", disk_type

        elif "is mounted:" in disk_info[i]:
            mount_flag = 0
            if len(tmp_disk_info) > 1:
                if tmp_disk_info[1]:
                    mount_flag = 1
                    log.info("Utils (%s), disk %d is mounted " % (dev, i))
            #print "usage is ", usage
        elif "mount paths:" in disk_info[i] and mount_flag:
            mount_path = tmp_disk_info[1]
            mount_flag = 0

        elif "partition table:" in disk_info[i]:
            partition_flag = 0
            if len(tmp_disk_info) > 1:
                if tmp_disk_info[1]:
                    partition_flag = 1
                    log.info("Utils (%s), disk %d has partitions " % (dev, i))
                #print "usage is ", usage
        elif "count:" in disk_info[i] and partition_flag:
            partition_num = tmp_disk_info[1]
            partition_flag = 0

        elif "type:" in disk_info[i]:
            if  len(tmp_disk_info) > 1:
                fs_type = tmp_disk_info[1]

            log.debug("Utils (%s), disk %d filesystem type is %s" % (dev,i,fs_type))
    rv = {'dev':dev, 'size':size, 'serial_num':serial_num, "model":model, "disk_type":disk_type, 'usage':usage, 'count':partition_num, 'vendor':vendor, 'temp':temp }

    return rv


#################################################
def disks():
    
    sas_dev_num = []
    disks = {}
    hosts = {}
    sas_dev = {}

    j = 0
    mount_flag = 0
    partition_flag = 0
    partition_num = '-'
    fs_type = 'unk'



    # --- FIND ALL AVAILABLE SCSI ATTACHED DEVICES, STORE IN OUT
    #res = subprocess.Popen(["lsscsi", "-t", "8"],  stdout=subprocess.PIPE)
    res = subprocess.Popen(["lsscsi", "-H"],  stdout=subprocess.PIPE)
    out = res.communicate()[0]
    all_scsi_dev = string.split(out)
    sl = len(all_scsi_dev)
    #print "all_scsi_dev = ", all_scsi_dev
    #print "sl = ", sl
    for i in range (1,sl,2):
        # --- if the devices uses the mpt2sas driver
        if all_scsi_dev[i] == "mpt2sas":
            val = all_scsi_dev[i-1]
            # --- extract the number from '[1]' string
            num_val = val[val.find('[')+1:val.find(']')]
            sas_dev_num.append(num_val)

    #print "sas_dev_num = ", sas_dev_num

    num_dev = len(sas_dev_num)

    a = b = ""
    for i in range(num_dev):

        res = subprocess.Popen(["lsscsi", "-t", sas_dev_num[i]],  stdout=subprocess.PIPE)
        # --- since we should have only 2 sas devices attached to driver at most:
        out = res.communicate()[0]
        if i == 0:
            a = string.split(out)
            #print "a = ", a
        else:
            b = string.split(out)
            #print "b = ", b

    # --- IF THE LENGHT OF B IS > 0 THEN IT WAS ASSIGNED, CONCATENATE OUTPUT
    if len(b) > 0:
        scsi_list = a + b
    elif len(a) > 0:
        scsi_list = a
    else:
        msg = "disks() no disk modules powered up"
        #print msg
        State.add_error_msg(msg)

        return(sas_dev)

    #print "scsi_list is ", scsi_list
    # --- SPLIT THE LIST BY SPACES SHOULD BE OF THE FORM
    '''
    [8:0:0:0]    disk    sas:0x4433221100000000          /dev/sdj
    [8:0:1:0]    disk    sas:0x4433221101000000          /dev/sdk
    [8:0:2:0]    disk    sas:0x4433221102000000          /dev/sdl
    [8:0:3:0]    disk    sas:0x4433221103000000          /dev/sdm
    [8:0:4:0]    disk    sas:0x4433221104000000          /dev/sdn
    [8:0:5:0]    disk    sas:0x4433221105000000          /dev/sdo
    [8:0:6:0]    disk    sas:0x4433221106000000          /dev/sdp
    [8:0:7:0]    disk    sas:0x4433221107000000          /dev/sdq
    '''

    # --- scsi_list number of items
    sll = len(scsi_list)
    # --- since the format is as above, every 4 is the devices
    num_available_disks = sll / 4
    log.debug("Utils  : num sll = %d,disks = %s" % (sll, num_available_disks))
    
    for i in range(0, sll, 4):

        # --- filter list in H:C:T:L format
        flist = scsi_list[i]
        flist = flist[flist.find("[")+1:flist.find("]")]
        #print "filter list - H:C:T:L = ", flist
        (host, ch, idnum, Lun) = string.split(flist,":")
        log.debug("Utils host %s, ch %s, id %s, Lun %s" % (host, ch, idnum, Lun))

        host = int(host)

        (ty, sas_address) = string.split(scsi_list[i+2],":")
        # --- the device number on the SAS controller, since it can be hex, convert from base 16 string format
        #print "sas_address = ", sas_address
        dev_num = int(sas_address[11],16)
        log.debug("Utils  dev_num = %d" % ( dev_num))
        
        # --- the device assignment /dev/sdx
        dev = scsi_list[i+3]
        log.info("Utils host %d, dev = %s, dev_number %d" % (host, dev, dev_num))
        
        # --- GET THE ADDIDITIONAL MANUFACUTURE INFORMATION 
        res = subprocess.Popen(["lsscsi", flist],  stdout=subprocess.PIPE)
        log.debug("Utils just called lsscsi ")
        out = res.communicate()[0]
        man_info = string.split(out)
        ml = len(man_info)
        if ml == 6:
            model = man_info[3]
            rev = man_info[4]
        elif ml == 7:
            model = man_info[4]
            rev = man_info[5]

        log.debug("Utils model = %s " % (model))
        # --- IF SAS DEVICES DOES NOT HAVE TH KEY FOR THE HOST, ADD IT BEFORE USING IT
        if not sas_dev.has_key(host):
            sas_dev[host] = {}
            log.info("Utils adding host %s" %(host))
        
        #ds = {}
        log.debug("Utils dev is %s " % (dev))
        d_np = disk_info_np(dev)
        log.debug("Utils returned from disk_info_np(%s)" % (dev))

        if d_np['count'] != "0":
            id_np = int(d_np['count'])
            #print "number of partitions are ", id_np
            log.debug(" Utils : number of partitions to check is %d " % (id_np))
            tmp_dev = dev+"1"
            log.debug("Utils calling disk_info_p(%s) " % (tmp_dev))
            d = disk_info_p(tmp_dev)
            log.debug("Utils return from disk_info_p(%s)" % (tmp_dev))
            # --- IF TWO PARTITIONS, THEN SCATTER GATHER IS ASSUMED
            if id_np == 2:
                tmp_dev = dev+"2"
                tmp_d = disk_info_p(tmp_dev)
                log.debug("modules_lib : second partition information meta_mount device  is %s meta_mount mount_pt %s " % (tmp_d['dev'] , tmp_d['mount_pt'] ))

        
            # --- ASSIGN THE DISK INFORMATION TO DEVICE NUMBER IDENTIFIER
            sas_dev[host][dev_num] = dict(d_np.items() + d.items())
        
        else:
            # --- if ther are no partitions then initialize id_np to 0 which it is.
            id_np = 0
            # --- ASSIGN THE DISK INFORMATION TO DEVICE NUMBER IDENTIFIER
            sas_dev[host][dev_num] = dict(d_np.items())

        
        if id_np == 2:
            sas_dev[host][dev_num]['mmpt'] = tmp_d['mount_pt']
            sas_dev[host][dev_num]['mmdev'] = tmp_d['dev']

        #log.debug( "Utils sas_device[%s][%s]  = %s" % (host, dev_num, sas_dev[host][dev_num]))
               
        # --- ASSIGN THE DISK INFORMATION TO DEVICE NUMBER IDENTIFIER
        log.debug("Utils assigning host to disks %d" % (host))
        #sas_dev[host][dev_num] = disks[dev_num]
        #print ""
        #log.debug("Utils sas_dev[%d] -> %s " % (host, sas_dev[host]))

        # --- IF THE USAGE IS SG AND THE MOUNT_POINT = '-', HOW TO DETERMINE IF THE META DATA PARTITION WAS MOUNTED
        #if y
    
    log.debug("Utils finished processing")
    #log.debug("Utils - disk(), keys are %d" % (sas_dev.keys()))
    for i in sas_dev.keys():
        #print "i is ", i
        log.debug("Utils returning sas_dev[%d] -> %s" % (i, sas_dev[i]))
        #print ""
    
    log.info("Utils all disks assigned returning")
    #log.debug("Utils:disks() : returning sas_device  = %s" % (sas_dev))
    return(sas_dev)

################################################################################
''' this routing takes the config_m6.xml parameter settings and compares it
to the actual disk information.  If they are the same, e.g. eMSN, it updates slot_status
to cslot values, and returns 1, for update.  If they are different it updates
what is on the disk and returns 0 for miss-match
'''
def slot_compare(cslot, slot_num):
    disk_msn = "-"
    #print cslot

    # --- DETERMINE IT THE MODULE POWERED UP IS INITIALIZED PROPERLY, AND MOUNT IF NOT
    (slt_st, disk_msn) = module_config(slot_num)

    # --- IF THE SLOT STATE FOR THE MODULE IS 1 IT HAS BEEN THROUGH MOD_INIT 
    if slt_st:
        log.info("slot_compare(%d) module has been properly initialized" %(slot_num))

        # --- IS the moduleNOT MOUNTED, MOUNT Partition
        slt_rv = module_mounted(slot_num)

        # --- GET THE eMSN from the disk module configure area.
    
    

    # --- ELSE IT HAS NOT BEEN INITIALIZED
    else:
        log.warning("slot_compare(%d) module has NOT been properly initialized " %(slot_num))
        disk_msn = "NA"
        log.debug("slot_compare(%d), assigning eMSN to %s  " %(slot_num, disk_msn))
    
    # --- IF THEY ARE THE SAME, SET SLOT_STATUS TO CSLOT.
    if cslot.eMSN == disk_msn:
        slot_status[slot_num] = cslot
        log.info("slot_status set to config_m6.xml values")
        return 1
    else:
        log.debug("slot_status comparison")
        # --- update the eMSN to MSN on the disk pack

        # --- IF NO MSN, PROVIDE WARNING THAT MODULE HAS NOT HAVE MSN

        # --- 
        return 0

##################################################################3
''' this routine processes the mark6 commands located in file 'name'
    It will verify the correctness of the command and return a 1 for success
    and a 0 if one of the commands failed.
'''
def process_cf(name):
    rv = 1
    cmd_file = open(name, 'r')
    log.info("process_cf - opened %s" % (name))
    # --- WE ASSUME THE COMMAND CAN ALL BE IN ONE LINE OR A COMMAND / LINE
    
    for l in cmd_file:
        if l != '\n':
            # --- LINE LIST IS BLANK
            ll = []
            ll = l.split(" ")
            # --- CREATE A COMMAND LINE WITHOUT SPACES
            line = ''.join(ll)

            # --- PASS THE LINE TO PARSER TO DETERMINE A COMMAND / QUEURY
            command = Parser.Parser().parse(line)
            log.debug("process_cf - returned from parse calling command.execute() command is %s" % (command))
            log.debug("process_cf - calling command.execute() ")
            response = command.execute()
            log.info("process_cf - returned from command.execute with %s" % response)
            
            # --- NEED CHECK HERE TO DETERMINE IF COMMAND WAS PROCESSED CORRECTLY
            #rv &= response_val
        else:
            continue
    log.debug("closing command file")
    cmd_file.close()

    return (rv)

###################################################################################
''' space determines all relevent information pertaining to disk space for the open group.
'''
def space(group, rate):
    log.debug("Utils - entered for open group %s at rate %d Mbps" % (group, rate))
    rv = 0
    remtime = 0
    remGB = 0
    totGB = 0

    # --- SEPERATE THE GROUP INTO A LIST, AND FIND INDIVIDUAL CAPACITY
    gl = list(group)
    if len(gl) != 0:
        for i in gl:
            remGB += module_libs.getremainingspace(group)
            totGB += State.slot_status[int(i)]['cap']

        remtime =  remGB * 8 / (int(rate) / 1000.)
        #remGB /= 1000000000
        #totGB /= 1000000000
        log.info("Utils : remGB %d" % remGB)
        log.info("Utils : totalGB %d" % totGB)
        rv = 0 
        # 
        
    return (rv, remtime, remGB, totGB)


def num_disks():

    # --- FIND ALL AVAILABLE SCSI ATTACHED DEVICES, STORE IN OUT
    #res = subprocess.Popen(["lsscsi", "-t", "8"],  stdout=subprocess.PIPE)
    res = subprocess.Popen(["lsscsi", "-H"],  stdout=subprocess.PIPE)
    out = res.communicate()[0]
    all_scsi_dev = string.split(out)
    sl = len(all_scsi_dev)
    #print "all_scsi_dev = ", all_scsi_dev
    #print "sl = ", sl
    for i in range (1,sl,2):
        # --- if the devices uses the mpt2sas driver
        if all_scsi_dev[i] == "mpt2sas":
            val = all_scsi_dev[i-1]
            # --- extract the number from '[1]' string
            num_val = val[val.find('[')+1:val.find(']')]
            sas_dev_num.append(num_val)

    #print "sas_dev_num = ", sas_dev_num

    num_dev = len(sas_dev_num)

    a = b = ""
    for i in range(num_dev):

        res = subprocess.Popen(["lsscsi", "-t", sas_dev_num[i]],  stdout=subprocess.PIPE)
        # --- since we should have only 2 sas devices attached to driver at most:
        out = res.communicate()[0]
        if i == 0:
            a = string.split(out)
            #print "a = ", a
        else:
            b = string.split(out)
            #print "b = ", b

    # --- IF THE LENGHT OF B IS > 0 THEN IT WAS ASSIGNED, CONCATENATE OUTPUT
    if len(b) > 0:
        scsi_list = a + b
    elif len(a) > 0:
        scsi_list = a
    else:
        msg = "disks() no disk modules powered up"
        #print msg
        State.add_error_msg(msg)

        return(sas_dev)

    #print "scsi_list is ", scsi_list
    # --- SPLIT THE LIST BY SPACES SHOULD BE OF THE FORM
    '''
    [8:0:0:0]    disk    sas:0x4433221100000000          /dev/sdj
    [8:0:1:0]    disk    sas:0x4433221101000000          /dev/sdk
    [8:0:2:0]    disk    sas:0x4433221102000000          /dev/sdl
    [8:0:3:0]    disk    sas:0x4433221103000000          /dev/sdm
    [8:0:4:0]    disk    sas:0x4433221104000000          /dev/sdn
    [8:0:5:0]    disk    sas:0x4433221105000000          /dev/sdo
    [8:0:6:0]    disk    sas:0x4433221106000000          /dev/sdp
    [8:0:7:0]    disk    sas:0x4433221107000000          /dev/sdq
    '''

    # --- scsi_list number of items
    sll = len(scsi_list)
    # --- since the format is as above, every 4 is the devices
    num_available_disks = sll / 4
    log.debug("Utils num sll = %d,disks = %s" % (sll, num_available_disks))
    
######################## TotalMem ############################
''' Returns the total Memory in the system. '''
def TotalMem():
    
    res =  subprocess.Popen(["grep", "MemTotal", "/proc/meminfo"],  stdout=subprocess.PIPE)
    out = res.communicate()[0]
    l = string.split(out)
    total_mem = l[1]
    mem_units = l[2]
    log.info("Utils total mem is %s in %s" % (total_mem, mem_units))
    if mem_units == 'kB':
        total_mem = str(int(total_mem) / 1000000)
        log.info("Utils total mem is %s in GB" % (total_mem))
    elif mem_units == 'MB':
        total_mem = str(int(total_mem) / 1000)
        log.info("Utils total mem is %s in GB" % (total_mem))
    

    return total_mem
######################## dplane_status #########################
''' This routines if dplane is active and sets the appropriate
status bit to say that the Mark6 is ready '''
def dplane_status():
    # --- CHECK TO SEE IF DPLANE IS RUNNING
    a = subprocess.Popen(["pidof", "dplane"], stdout=subprocess.PIPE)
    l = string.split(a.communicate()[0], '\n')
    rv = 0

    if l[0] != '':
        log.info("DPLANE is running %s" % (l[0]))
        State.c_status |= 0x100
        log.info("Utils - changing status to dplane operational ")
        rv = 1
    else:
        log.warning("DPLANE HAS NOT BEEN STARTED, COMMANDS THAT REQUIRE COMMUNICATIONS TO IT WILL NOT BE PROCESSED")
        # --- DO NOT HAVE TO CHANGE STATUS SINCE IT IS DEFAULT 0
        State.c_status &= 0xffff0ff

    return rv

###########################################3
#  verify group, checks all slot_status group setting and verifies they are the same.
# returns a 1 for success, 0 for failure
#
def verify_group(vg_lparam):
    rv = []
    rv = 1
    lvg = len(vg_lparam)
    #print "lvg = ", lvg
    sslots = ''.join(vg_lparam)
    #print 'sslots ', sslots
    
    gr_cnt = 0           # --- GROUP COUNTER IN LPARAM
    for i in vg_lparam:
        ii = int(i)
        if ii in [1,2,3,4]:
            # --- READ THE GROUP OF SLOT
            slots_group = State.slot_status[ii]['group']
            log.debug("Utils : slot_group is %s" % slots_group)
            if "-" in slots_group:
                log.info("Utils : the group has a missing module or incomplete %s" % slots_group)
                return 0
            elif len(sslots) == 1 and slots_group == i:
                # --- THE GROUP SHOULD EQUAL THE SLOT
                log.info("Utils : the group is one module and is the slot %s" % i)
                return 1
            else:
                cnt = 0
                for j in range(0, lvg):
                    if j != gr_cnt: 
                        #print "j = ", j
                        #print "vg_lparam =", vg_lparam
                        slot_cmp = int(vg_lparam[j])
                        #print slot_cmp
                        log.debug("Utils : comparing to %s" % State.slot_status[slot_cmp]['group'])
                        if slots_group == State.slot_status[slot_cmp]['group']:
                            cnt+=1
                    # --- increment the counter since j == i means it in the group
                    else:  cnt+=1

                if cnt == lvg:
                    log.info("Utils : the group is complete %d" % cnt)
                    return 1
                else:
                    log.info("Utils : the group is incomplete %d" % cnt)
                    return 0
            gr_cnt += 1
        else:
            return 0

    return 0
    
######################################################
# si_start() takes the slot and gives back the location of the first disk information.  The HBA
#            controller labels the disk 0-15.
def si_start(islot):
    #print "si_start -> ", islot
    if type(islot) != 'int':
        islot = int(islot)

        dev_start = (islot % 2)
        
        if dev_start == 0: return 8
        else: return 0

##############################################################################
# --- UPDATE THE NUMBER OF DEVICES FOR RECORDING
def open_storage_dev(osd_gref):
    log.debug("Utils : entered for group_ref %s" % osd_gref)
    losd_gref = list(osd_gref)
    #print "losd_gref ", losd_gref

    tot_devs = 0

    if len(losd_gref) != 0:
        for i in losd_gref:
            #print "i is ", i
            ii = int(i)
            tot_devs += State.slot_status[ii]['rdisks']
    else:
        msg = "Utils : received invalid gref %s" % osd_gref
        State.add_error_msg(msg)
        
    log.debug("Utils : returning num of open storage devices %d" % tot_devs)

    return tot_devs

##############################################################################
# --- UPDATE THE DEVICE LIST FOR DPLANE OF DEVICES FOR RECORDING
def open_record_dev(ord_gref):
    log.debug("Utils : entered with group_ref%s" % ord_gref)
    lord_gref = list(ord_gref)
    fmask = mask = 0
    dev_list = []
    if lord_gref != 0:
        #for i in range (1, len(lord_gref)+1):
        for i in lord_gref:
            ii = int(i)
            # --- NUMBER OF DEVICES TO EXPECT
            devs = State.slot_status[ii]['rdisks']
            
            dev_start = si_start(ii)
            dev_end = dev_start + devs
            log.debug("Utils : start %d, end %d " % (dev_start, dev_end))
            for j in range (dev_start, dev_end):
                dev = State.slot_info[ii][j]['dev']
                #print "dev1 ",dev
                # --- DEV IS IN FORM /DEV/SDX1
                dev = dev[dev.rfind("/")+1:]
                #print "dev2 ",dev
                # --- append THE SDX1 PORTION
                dev_list.append(dev)
                #print "devs_list ", dev_list
                # --  TO CALCULATE FMASK
                #if j%8:
                pos = 1 << (j%8)
                #else:
                #    pos = 1

                mask |= pos
                #print "mask is ", mask

            # position of fmask is based on gref slot
            State.fmask |= mask << 8 * (ii-1)
            #print "State.fmask ", State.fmask
            final_mask = bin(State.fmask)
            final_mask = final_mask[final_mask.find('b')+1:]
        #print "final fmask is %032s" % (final_mask)

    else:
        msg = "Utils : received invalid gref %s" % ord_gref
        State.add_error_msg(msg)
        
    log.debug("Utils : returning list of storage nodes %s" % dev_list)

    return dev_list
##############################################################################
# --- UPDATE THE fmask, DEV_LIST EXISTS, BUT HAD A DEVICE REMOVED.  DETERMINE LOCATION
# AND REMOVE FROM FMASK, expects the Device removed to be passed.
def update_record_fmask(rm_dev):
    log.debug("Utils : entered with device to be removed %s" % rm_dev)
    fmask = State.fmask

    if len(dev) != 0 and open_group['type'] == 'sg':
        #for i in range (1, len(lord_gref)+1):
        for i in State.open_group['slot']:
            ii = int(i)
            #print "ii is ", ii
            # --- NUMBER OF DEVICES TO EXPECT
            devs = State.slot_status[ii]['rdisks']
            
            dev_start = si_start(ii)
            dev_end = dev_start + devs
            log.debug("Utils : start %d, end %d " % (dev_start, dev_end))
            for j in range (dev_start, dev_end):
                dev = State.slot_info[ii][j]['dev']
                #print "dev1 ",dev
                # --- DEV IS IN FORM /DEV/SDX1
                dev = dev[dev.rfind("/")+1:]
                #print "dev2 ",dev
                if dev != rm_dev:
                    # --- append THE SDX1 PORTION
                    dev_list.append(dev)
                    #print "devs_list ", dev_list
                    # --  TO CALCULATE FMASK
                    pos = 1 << (j%8)
                    mask |= pos
                    #print "mask is ", mask
                else:
                    # --- decrement the number of registered disks
                    State.slot_status[ii]['rdisks'] -= 1 
                    log.warning("Utils : slot %s had a failed disk, reducing the number of registered disks" %  i)

            # position of fmask is based on gref slot
            State.fmask |= mask << 8 * (ii-1)
            #print "State.fmask ", State.fmask
            final_mask = bin(State.fmask)
            final_mask = final_mask[final_mask.find('b')+1:]
        #print "final fmask is %032s" % (final_mask)

    else:
        msg = "Utils : called with no device passed  %s" % dev
        State.add_error_msg(msg)
        
    log.debug("Utils : returning list of storage nodes %s" % dev_list)

    return dev_list

################################################################
# --- THIS ROUTINE, EXPECTS A LIST OF COMMANDS USED WITH POPEN,
#    EXECUTES IT, AND RETURNS 0/1 AND OUPUT IF ERROR
def linux_cmd(cmd_list):

    log.debug("Utils : executing command %s" % (cmd_list))
    res = subprocess.Popen(cmd_list, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    out, err = res.communicate()
    if len(err) > 0:
        rv = 0
        msg = err
    else : 
        rv = 1
        msg = out

    return (rv, msg)

################################################################
# --- THIS ROUTINE, EXPECTS A LIST OF COMMANDS USED WITH POPEN,
#    EXECUTES IT, AND RETURNS 0/1 AND OUPUT IF ERROR
def linux_cmd_shell(cmd_list):

    log.debug("Utils : executing command %s" % (cmd_list))
    res = subprocess.Popen(cmd_list, shell=True, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    out, err = res.communicate()
    if len(err) > 0:
        rv = 0
        msg = err
    else : 
        rv = 1
        msg = out

    return (rv, msg)

################################################################
# --- THIS ROUTINE, EXPECTS TWO LISTS OF COMMANDS USED WITH POPEN,
#    EXECUTES A cmd1 | cmd2
def linux_cmd_pipe(cmd1, cmd2):

    log.debug("Utils : executing command %s | %s" % (cmd1, cmd2))
    p1 = subprocess.Popen(cmd1, stdout=subprocess.PIPE)
    #out, err = p1.communicate()
    #if len(err) > 0:
    #    rv = 0
    #    msg = err
    #else : 
    p2 = subprocess.Popen(cmd2, stdin=p1.stdout, stdout=subprocess.PIPE)
    out = p2.communicate()[0]

    rv = 1
    msg = out

    return (rv, msg)

################################################################
# --- ASSUMES THE GROUP HAS BEEN VERIFIED, NOW SEEING IF ALL MEMBERS OF THE GROUP ARE READABLE
def readable(lg):
    log.debug("Entered readable")
    rv = 1
    llg = len(lg)
    #print "lg = ", llg
    sslots = ''.join(lg)
    #print 'sslots ', sslots
    
    gr_cnt = 0           # --- GROUP COUNTER IN LPARAM
    for i in lg:
        ii = int(i)
        # --- READ THE GROUP OF SLOT
        slot_status = State.slot_status[ii]['status1']
        log.debug("Utils : slot_status is %s" % slot_status)
        if slot_status == 'open' or slot_status == 'closed':
            gr_cnt += 1
            #print "gr_cnt ", gr_cnt

    if gr_cnt == llg:
        return 1
    else:
        return 0
def get_m6cc():

    if State.m6cc_pid != 0:
        cmd = ['ps', '-p', str(State.m6cc_pid)]
        (rv, out) = linux_cmd(cmd)
        if rv == 0:
            msg = out
            State.add_error_msg(msg)
            rc='4'
        else:
            rc='0'
        #print "out = ", out
            ps_data = out.split("\n")
        # --- IF THE LEN > 
            if len(ps_data) > 1:
            #print ps_data[1]
                if ps_data[1] != '':
                    State.m6cc_state = 'active'
                else:
                    # --- IF THE STATE WAS ACTIVE, BUT NO ps EXISTS THEN COMPLETED SUCCESSFULLY
                    if State.m6cc_state == 'active':
                        State.m6cc_state = 'complete'
                    # --- ELSE THE STATE WAS SET ELSE WHERE , E.G. ABORT.
                        
            else:
                rc = '1'
    else:
        cmd1 = ['ps', 'auxw']
        cmd2 = ['grep', 'M6_CC']
        (rv, out) = linux_cmd_pipe(cmd1, cmd2)

        if rv == 0:
            msg = out
            State.add_error_msg(msg)
            rc='4'
        else:
            rc='0'
            #print "out = ", out
            ps_data = out.split("\n")
            #print "ps_data ", ps_data
        # --- IF THE LEN > 
            for l in ps_data:
                if "grep" in l:
                    log.debug("skipping line %s ", l)
                elif "M6_CC" in l:
                    State.m6cc_state = 'active'
                    rc = '1'
                    log.debug("MCC in %s must be active" % (l))
    
    #print "rc = ", rc
    return rc
    
#######################################################################
# --- SINCE SPACES ARE REMOVED WHEN A COMMAND IS RECEIVED, THERE ARE 
#  CERTAIN INSTANCES WHERE THEY ARE REQUIRED.  THIS ROcUTINE LOOKS FOR EVERY
#  OTHER " AND INSERTS A SPACE AFTER IT, AND RETURNS THE NEW STRING
def insert_spaces(line):

    num_char = len(line)
    
    loc = 0
    
    while loc <= num_char:
        x = line.find('"')

    return line

