#!/usr/bin/python
import subprocess
import string
import sys

orig_sas = {8:{'/dev/sdp': 7, '/dev/sdq': 5, '/dev/sdn': 4, '/dev/sdo': 6, '/dev/sdl': 15, '/dev/sdm': 14, '/dev/sdj': 13, '/dev/sdk': 12, '/dev/sdh': 3, '/dev/sdi': 2, '/dev/sdf': 1, '/dev/sdg': 0, '/dev/sdd': 9, '/dev/sde': 11, '/dev/sdb': 8, '/dev/sdc': 10}}

sas_dev= {8:{0: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HAY%0002_4', 'temp': '33C', 'dev': '/dev/sdg1', 'serial_num': 'WD-WMAWP0422279', 'mmdev': '/dev/sdg2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD2002FAEX-007BA0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '2000298180608'}, 1: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HAY%0002_5', 'temp': '33C', 'dev': '/dev/sdf1', 'serial_num': 'WD-WMAWP0482701', 'mmdev': '/dev/sdf2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD2002FAEX-007BA0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '2000298180608'}, 2: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HAY%0002_6', 'temp': '33C', 'dev': '/dev/sdi1', 'serial_num': 'WD-WMAWP0482409', 'mmdev': '/dev/sdi2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD2002FAEX-007BA0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '2000298180608'}, 3: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HAY%0002_7', 'temp': '30C', 'dev': '/dev/sdh1', 'serial_num': 'WD-WMAWP0457555', 'mmdev': '/dev/sdh2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD2002FAEX-007BA0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '2000298180608'}, 4: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HAY%0002_0', 'temp': '32C', 'dev': '/dev/sdn1', 'serial_num': 'WD-WMAWP0499898', 'mmdev': '/dev/sdn2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD2002FAEX-007BA0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '2000298180608'}, 5: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HAY%0002_1', 'temp': '33C', 'dev': '/dev/sdq1', 'serial_num': 'WD-WMAWP0483337', 'mmdev': '/dev/sdq2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD2002FAEX-007BA0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '2000298180608'}, 6: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HAY%0002_2', 'temp': '32C', 'dev': '/dev/sdo1', 'serial_num': 'WD-WMAWP0482551', 'mmdev': '/dev/sdo2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD2002FAEX-007BA0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '2000298180608'}, 7: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HAY%0002_3', 'temp': '31C', 'dev': '/dev/sdp1', 'serial_num': 'WD-WMAWP0500326', 'mmdev': '/dev/sdp2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD2002FAEX-007BA0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '2000298180608'}, 8: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HSG6+002_0', 'temp': '32C', 'dev': '/dev/sdb1', 'serial_num': 'WD-WCATR5861461', 'mmdev': '/dev/sdb2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD1002FAEX-00Z3A0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '1000103477248'}, 9: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HSG6+002_1', 'temp': '35C', 'dev': '/dev/sdd1', 'serial_num': 'WD-WCATR5844317', 'mmdev': '/dev/sdd2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD1002FAEX-00Z3A0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '1000103477248'}, 10: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HSG6+002_2', 'temp': '34C', 'dev': '/dev/sdc1', 'serial_num': 'WD-WCATR5862093', 'mmdev': '/dev/sdc2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD1002FAEX-00Z3A0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '1000103477248'}, 11: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HSG6+002_3', 'temp': '32C', 'dev': '/dev/sde1', 'serial_num': 'WD-WCATR5882880', 'mmdev': '/dev/sde2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD1002FAEX-00Z3A0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '1000103477248'}, 12: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HSG6+002_4', 'temp': '34C', 'dev': '/dev/sdk1', 'serial_num': 'WD-WCATR5883759', 'mmdev': '/dev/sdk2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD1002FAEX-00Z3A0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '1000103477248'}, 13: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HSG6+002_5', 'temp': '35C', 'dev': '/dev/sdj1', 'serial_num': 'WD-WCATR5835640', 'mmdev': '/dev/sdj2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD1002FAEX-00Z3A0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '1000103477248'}, 14: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HSG6+002_6', 'temp': '32C', 'dev': '/dev/sdm1', 'serial_num': 'WD-WCATR5873858', 'mmdev': '/dev/sdm2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD1002FAEX-00Z3A0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '1000103477248'}, 15: {'count': '2', 'fs': 'xfs', 'vendor': 'ATA', 'part_label': 'HSG6+002_7', 'temp': '33C', 'dev': '/dev/sdl1', 'serial_num': 'WD-WCATR5873971', 'mmdev': '/dev/sdl2', 'mount_pt': '-', 'usage': 'sg', 'model': 'WDC WD1002FAEX-00Z3A0 ', 'disk_type': 'Rotating', 'part_num': '1', 'mmpt': '-', 'size': '1000103477248'}}}

#################################################
def disks():

    global orig_sas

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

    print "sas_dev_num = ", sas_dev_num

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
    print("Utils  : num sll = %d,disks = %s" % (sll, num_available_disks))
    
    for i in range(0, sll, 4):

        # --- filter list in H:C:T:L format
        flist = scsi_list[i]
        flist = flist[flist.find("[")+1:flist.find("]")]
        #print "filter list - H:C:T:L = ", flist
        (host, ch, idnum, Lun) = string.split(flist,":")
        #print("Utils host %s, ch %s, id %s, Lun %s" % (host, ch, idnum, Lun))

        host = int(host)

        (ty, sas_address) = string.split(scsi_list[i+2],":")
        # --- the device number on the SAS controller, since it can be hex, convert from base 16 string format
        #print "sas_address = ", sas_address
        dev_num = int(sas_address[11],16)
        
        # --- the device assignment /dev/sdx
        dev = scsi_list[i+3]
        
        # --- GET THE ADDIDITIONAL MANUFACUTURE INFORMATION 
        res = subprocess.Popen(["lsscsi", flist],  stdout=subprocess.PIPE)
        out = res.communicate()[0]
        man_info = string.split(out)
        ml = len(man_info)
        if ml == 6:
            model = man_info[3]
            rev = man_info[4]
        elif ml == 7:
            model = man_info[4]
            rev = man_info[5]

        print("Utils host:%d, dev:%s, dev_number:%d, model:%s" % (host, dev, dev_num, model))

        # --- IF SAS DEVICES DOES NOT HAVE TH KEY FOR THE HOST, ADD IT BEFORE USING IT
        if not sas_dev.has_key(host):
            sas_dev[host] = {}
            print ("Utils adding dev %s" %(host))
        
        if not sas_dev[host].has_key(dev):
            sas_dev[host][dev] = dev_num

    print("Utils finished processing")
    #print("Utils - disk(), keys are %d" % (sas_dev.keys()))
    for i in sas_dev.keys():
        #print "i is ", i
        print("Utils returning sas_dev[%d] -> %s" % (i, sas_dev[i]))
        #print ""
    
    print("Utils all disks assigned returning")
    #print("Utils:disks() : returning sas_device  = %s" % (sas_dev))
    return(sas_dev)

def main():
    global orig_sas
    global sas_dev
    slot_info = {1:{}, 2:{}, 3:{}, 4:{}} 
    host_keys = []
    slot_disks = {1:"off", 2:"off", 3:"off", 4:"off"}
    print "in disks"
    sas_state = disks()
    if sas_state == orig_sas:
        print "no change"
    else:
        print "Have disks been powered off?"
        num_pwr_off = 0
        for i in orig_sas.keys():
            for j in orig_sas[i].keys():
                if not sas_state[i].has_key(j):
                    print "dev powered off i-%s j-%s" % (i, j)
                    sas_dev_key = int(orig_sas[i][j])
                    ii =int(i)
                    print "removing from sas_dev[%d][%d] " % (ii,sas_dev_key)
                    del sas_dev[ii][sas_dev_key]
                    print "removing from orig_sas"
                    del orig_sas[i][j]
                    num_pwr_off += 1
                
                    
        print "---Have new disks been powered on?---"
        num_pwr_on = 0
        for i in sas_state.keys():
            for j in sas_state[i].keys():
                if not orig_sas[i].has_key(j): 
                    print "Yes new device powered on for i-%s j-%s" % (i, j)
                    orig_sas = {i:j}
                    num_pwr_on += 1                 
        
        print "Determine if a disk module was completely powered off"
        if num_pwr_off or num_pwr_on:
            print " validate power off /on"

            msn_state = sas_dev
            host_keys = msn_state.keys()
            host_keys.sort()            
            # --- ASSIGN THE DISK TO THE SPECIFIC SLOT THEY ARE IN:  HERE SLOT REFERS TO DEV NUMBER
            for slot, disk in msn_state.iteritems():
                if slot == host_keys[0]:
                    for i in disk:
                        print("State msn_state[%d][%i] = %s" %(slot,i, msn_state[slot][i]))
                        #print msn_state[slot][i]
                        if i < 8:
                            slot_info[1][i] = msn_state[slot][i]
                            slot_disks[1] = "on"
                        else:
                            slot_info[2][i] = msn_state[slot][i]                        
                            slot_disks[2] = "on"
                else:
                    for i in disks:
                        print("State msn_state[%d][%i] = %s" %(slot,i, msn_state[slot][i]))
                        if i < 8:
                            slot_info[3][i] = msn_state[slot][i] 
                            slot_disks[3] = "on"
                        else:
                            slot_info[4][i] = msn_state[slot][i]                 
                            slot_disks[4] = "on"

        print "slot_info ", slot_info
        print "slot_disks = ", slot_disks

        print "sas_dev is now %s" % sas_dev
        print " "                    
        print "sas info is now %s" % orig_sas


if __name__ == '__main__':
    #disk_state()
    main()
    sys.exit(0)
