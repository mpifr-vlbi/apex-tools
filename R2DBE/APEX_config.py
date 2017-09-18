#!/usr/bin/python
"""
Remotely configure a R2DBE after it has booted.

Usage: ./APEX_config.py [-n|--noreconf] [-t|--threadIDs 0,1] [-s|--stationIDs Ap,Ar]
                        [-a|--arp <IP[4],MAC>] [<hostname>]

The default R2DBE host name is r2dbe-1. The tcpborphserver3 must
be running on the R2DBE host, usually this is automatically the
case after the R2DBE has successfully booted up.

Options:
  --noreconf   skip reconfiguring the FPGA i.e. do not reprogram firmware (default: do reconfigure)
  --threadIDs  a pair of comma-separated integer VDIF thread IDs to use on 10GbE ports (default: 0,1)
  --stationIDs a pair of 2-letter VDIF station IDs to use on 10GbE ports (default: AR,Ar)
  --arp        a pair of last-part-of-IPv4 and a MAC address, e.g., --arp 17,00:60:dd:44:46:38,
               this argument can be specified multiple times to add more R2DBE ARP table entries
"""
import adc5g, corr
from time import sleep
from datetime import datetime, time, timedelta
import netifaces
import sys

is_test = 0

def usage():
    print __doc__

def MACc2int(mac):
    mac_val = mac.replace(':','')
    mac_val = int(mac_val,base=16)
    return mac_val

def IP2intarray(ip):
    ip_vals = ip.split('.')
    ip_vals = [int(v) for v in ip_vals]
    return ip_vals

# Defaults
r2dbe_hostname = 'r2dbe-1'
dest_udp_port = 4001
do_reprogram = True    # upload the FPGA firmware again
station_id_0  = 0x4152 # 'AR' for APEX R2DBE
station_id_1  = 0x4172 # 'Ar' to discern other IF
thread_id_0 = 0
thread_id_1 = 1

# Default ARP table : all broadcast
arp = [0xffffffffffff] * 256  # last part of IP address <--> MAC
do_local_ARP = True           # copy MACs from local computer interfaces
local_ifaces = ['eth2','eth3','eth4','eth5']

# Command line options
args = sys.argv[1:]
while len(args)>0 and args[0][0]=='-':
    if args[0]=='-n' or args[0]=='--noreconf':
        do_reprogram = False
        args = args[1:]
    elif args[0]=='-h' or args[0]=='--help':
        usage()
        sys.exit(0)
    elif args[0]=='-t' or args[0]=='--threadIDs':
        tids = args[1].split(',')
        if len(tids) != 2:
            print('Error: expected two comma-separated values for --threadIDs, got %s' % (args[1]))
            sys.exit(0)
        thread_id_0 = int(tids[0])
        thread_id_1 = int(tids[1])
        args = args[2:]
    elif args[0]=='-s' or args[0]=='--stationIDs':
        sids = args[1].split(',')
        if (len(sids) != 2) or (len(sids[0]) != 2 or len(sids[1]) != 2):
            print('Error: expected two two-letter comma-separated values for --stationIDs, got %s' % (args[1]))
            sys.exit(0)
        station_id_0 = ord(sids[0][0])*256 + ord(sids[0][1])
        station_id_1 = ord(sids[1][0])*256 + ord(sids[1][1])
        args = args[2:]
    else:
        print('Error: unknown arg %s' % (args[0]))
        sys.exit(0)
if len(args) not in [0,1]:
    print('Error: too many args, %s was unexpected' % (str(args[1:])))
    sys.exit(0)
if len(args)==1:
    r2dbe_hostname = args[0]

print ('Parsed args : t0=%d t1=%d s0=%X s1=%X host=%s' % (thread_id_0,thread_id_1,station_id_0,station_id_1,r2dbe_hostname))

# Derive a default R2DBE ARP table using local IPs and MACs
if do_local_ARP:
    for iface in local_ifaces:
        ip_i4 = IP2intarray( netifaces.ifaddresses(iface)[netifaces.AF_INET][0]['addr'] )[3]
        mac = MACc2int( netifaces.ifaddresses(iface)[netifaces.AF_LINK][0]['addr'] )
        arp[ip_i4] = mac
        print ('Auto-ARP : dst %s : IPs x.x.x.%d mapped to MAC %012x' % (iface,ip_i4,mac))


sys.exit(0)

# Connect
roach2 = corr.katcp_wrapper.FpgaClient(r2dbe_hostname)
roach2.wait_connected()
if do_reprogram:
    print 'Programming the R2DBE FPGA with firmware file r2dbe_rev2.bof...'
    roach2.progdev('r2dbe_rev2.bof') # JanW: must do 'gzip -d r2dbe_rev2.bof.gz' manually 
    roach2.wait_connected()
else:
    print 'Skipping R2DBE FPGA reconfiguration step, proceeding directly to settings change.'

# Set data mux to ADC
roach2.write_int('r2dbe_data_mux_0_sel', 1)
roach2.write_int('r2dbe_data_mux_1_sel', 1)

# Calibrate ADCs
print 'Calibrating ADC clock phase...'
adc5g.set_test_mode(roach2, 0)
adc5g.set_test_mode(roach2, 1)
adc5g.sync_adc(roach2)
opt, glitches = adc5g.calibrate_mmcm_phase(roach2, 0, ['r2dbe_snap_8bit_0_data',])
print 'Optimum and glitches for tested phase offsets on adc0:'
print opt, glitches
opt, glitches = adc5g.calibrate_mmcm_phase(roach2, 1, ['r2dbe_snap_8bit_1_data',])
print 'Optimum and glitches for tested phase offsets on adc1:'
print opt, glitches
adc5g.unset_test_mode(roach2, 0)
adc5g.unset_test_mode(roach2, 1)

# Set 10 gbe vals
print 'Configuring 10 GbE...'

# Src and Dest IP
ip_base = [192,168,1,1]
for ip_part, name in ((2,'tengbe_0'), (3,'tengbe_1')):

    # VDIF dest ip   = 10.10.1.<xx>
    # VDIF source ip = 10.10.1.<xx+4>
    # tengbe_0 --> 10.10.1.1 (mark6-4031 eth2),  tengbe_1 --> 10.10.1.17 (mark6-4031 eth3)
    # tengbe_2 not in current firwmare,          tengbe_3 not in current firmware

    dest_ip = (ip_base[0]<<24) + (ip_base[1]<<16) + (ip_base[2]<<8) + (ip_part + 0)
    src_ip  = (ip_base[0]<<24) + (ip_base[1]<<16) + (ip_base[2]<<8) + (ip_part + 4)
    src_mac = (2<<40) + (2<<32) + 20 + src_ip

    roach2.config_10gbe_core('r2dbe_' + name + '_core', src_mac, src_ip, 4000, arp)
    roach2.write_int('r2dbe_' + name + '_dest_ip', dest_ip)
    roach2.write_int('r2dbe_' + name + '_dest_port', dest_udp_port)

    # reset tengbe (this is VITAL)
    roach2.write_int('r2dbe_' + name + '_rst', 1)
    roach2.write_int('r2dbe_' + name + '_rst', 0)
    
# arm the one pps
roach2.write_int('r2dbe_onepps_ctrl', 1<<31)
roach2.write_int('r2dbe_onepps_ctrl', 0)
sleep(2)

#######################################
# set headers
#######################################
#changed 27/7/2015 to merge new r2bde_start.py with new clock epoch JB
# calculate reference epoch
utcnow = datetime.utcnow()
ref_start = datetime(2000,1,1,0,0,0)

nyrs = utcnow.year - ref_start.year 
ref_ep_num = 2*nyrs+1*(utcnow.month>6)

ref_ep_date = datetime(utcnow.year,6*(utcnow.month>6)+1,1,0,0,0) 
# date ofstart of epoch 01 July 2015
print 'Using VDIF reference epoch %d (%s)' % (ref_ep_num, str(ref_ep_date))

##############
#   W0
##############


delta       = utcnow-ref_ep_date
sec_ref_ep  = delta.seconds + 24*3600*delta.days

# to check
nday = sec_ref_ep/24/3600

#secs_ref_ep_nday, number of days since reference epoch began
#print delta.total_seconds()
#print 'secs since ref ep: %d' %sec_ref_ep
#print 'days since ref ep: %d' %nday
roach2.write_int('r2dbe_vdif_0_hdr_w0_reset',1)
roach2.write_int('r2dbe_vdif_0_hdr_w0_reset',0)
roach2.write_int('r2dbe_vdif_1_hdr_w0_reset',1)
roach2.write_int('r2dbe_vdif_1_hdr_w0_reset',0)
roach2.write_int('r2dbe_vdif_0_hdr_w0_sec_ref_ep',sec_ref_ep)
roach2.write_int('r2dbe_vdif_1_hdr_w0_sec_ref_ep',sec_ref_ep)

#############
#   W1
#############
#print "reference epoch number: %d" %ref_ep_num
roach2.write_int('r2dbe_vdif_0_hdr_w1_ref_ep',ref_ep_num)
roach2.write_int('r2dbe_vdif_1_hdr_w1_ref_ep',ref_ep_num)


#############
#   W2
#############

# nothing to do


############
#   W3 
############
roach2.write_int('r2dbe_vdif_0_hdr_w3_thread_id', thread_id_0)
roach2.write_int('r2dbe_vdif_1_hdr_w3_thread_id', thread_id_1)

# convert chars to 16 bit int
#st0 = ord(station_id_0[0])*2**8 + ord(station_id_0[1])
#st1 = ord(station_id_1[0])*2**8 + ord(station_id_1[1])

roach2.write_int('r2dbe_vdif_0_hdr_w3_station_id', station_id_0)
roach2.write_int('r2dbe_vdif_1_hdr_w3_station_id', station_id_1)

############
#   W4
############

#eud_vers = 0x02

#w4_0 = eud_vers*2**24 + pol_block0
#w4_1 = eud_vers*2**24 + pol_block1

#roach2.write_int('r2dbe_vdif_0_hdr_w4',w4_0)
#roach2.write_int('r2dbe_vdif_1_hdr_w4',w4_1)

############
#   W5
############

# the offset in FPGA clocks between the R2DBE internal pps
# and the incoming GPS pps

############
#   W6
############

#  PSN low word, written by FPGA to VDIF header

###########
#   W7
############

# PSN high word, written by FPGA to VDIF header


# select test data 
roach2.write_int('r2dbe_vdif_0_test_sel', is_test)
roach2.write_int('r2dbe_vdif_1_test_sel', is_test)

# use little endian word order
roach2.write_int('r2dbe_vdif_0_little_end', 1)
roach2.write_int('r2dbe_vdif_1_little_end', 1)

# reverse time order (per vdif spec)
roach2.write_int('r2dbe_vdif_0_reorder_2b_samps', 1)
roach2.write_int('r2dbe_vdif_1_reorder_2b_samps', 1)

# set to test-vector noise mode
roach2.write_int('r2dbe_quantize_0_thresh', 12)	
roach2.write_int('r2dbe_quantize_1_thresh', 12)
# JanW: -7 dBm over 0-1.5 GHz band input to R2DBE
#  thresh=16 produces 10%:41%:41%:8%
#  thresh=12 produces 16%:35%:35%:14%

# set threshold automatically
# get snapshot of 8-bit data
print 'Setting 8-bit -> 2-bit quantization threshold levels...'
x8_0 = adc5g.get_snapshot(roach2,'r2dbe_snap_8bit_0_data')
L    = len(x8_0)
print 'Auto-thresholding input 0 using %d samples' % (L)
y    = sorted(x8_0)
Lt   = int(L*0.16)
th1  = abs(y[Lt-1])
Lt2  = int(L*0.84)
th2  = abs(y[Lt2-1])
th   = (th1+th2)/2
roach2.write_int('r2dbe_quantize_0_thresh', th)
print 'r2dbe_quantize_0_thresh : %d' % (th)

x8_1 = adc5g.get_snapshot(roach2,'r2dbe_snap_8bit_1_data')
L    = len(x8_1)
print 'Auto-thresholding input 1 using %d samples' % (L)
y    = sorted(x8_1)
Lt   = int(L*0.16)
th1  = abs(y[Lt-1])
Lt2  = int(L*0.84)
th2  = abs(y[Lt2-1])
th = (th1+th2)/2
roach2.write_int('r2dbe_quantize_1_thresh', th)
print 'r2dbe_quantize_1_thresh : %d' % (th)

# arm the one pps
#JanW: Laura's SPT code version moved this to right after IP/MAC config
#roach2.write_int('r2dbe_onepps_ctrl', 1<<31)
#roach2.write_int('r2dbe_onepps_ctrl', 0)

# must wait to set the enable signal until pps signal is stable
sleep(2)
print 'Enabling VDIF stream...'
roach2.write_int('r2dbe_vdif_0_enable', 1)
roach2.write_int('r2dbe_vdif_1_enable', 1)
