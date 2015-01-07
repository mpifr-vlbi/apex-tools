#!/usr/bin/python
import adc5g, corr
from time import sleep
from datetime import datetime, time, timedelta
import sys

is_test = 0
dest_udp_port = 4001

station_id_0  = 0x4152 # 'AR' for APEX R2DBE
station_id_1  = 0x4172 # 'Ar' to discern other IF

# set thread id for both blocks
# perhaps thread is always 0?
thread_id_0 = 0
thread_id_1 = 0

roach2 = corr.katcp_wrapper.FpgaClient('r2dbe-1')
roach2.wait_connected()

if len(sys.argv)==1:
	print 'Programming the R2DBE FPGA with firmware file r2dbe_rev2.bof...'
	roach2.progdev('r2dbe_rev2.bof') # JanW: must do 'gzip -d r2dbe_rev2.bof.gz' manually 
	roach2.wait_connected()
else:
	print 'Skipping R2DBE FPGA reconfiguration step, proceeding directly to settings change.'

# set data mux to ADC
roach2.write_int('r2dbe_data_mux_0_sel', 1)
roach2.write_int('r2dbe_data_mux_1_sel', 1)

# calibrate ADCs
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

# set 10 gbe vals
print 'Configuring 10 GbE...'

# APEX mark6-4031
#  netmask 255.255.255.240 to use lower 4-bit for each net
#  eth2 : 10.10.1.1  -- 10.10.1.14 net .0  bcast .15
#  eth3 : 10.10.1.17 -- 10.10.1.30 net .16 bcast .31
#  eth4 : 10.10.1.33 -- 10.10.1.46 net .32 bcast .47
#  eth5 : 10.10.1.49 -- 10.10.1.62 net .48 bcast .63

# ARP table : MAC <--> last part of IP address
arp     = [0xffffffffffff] * 256  # defaults
arp[1]  = 0x0060dd440b74     # mark6-4031 eth2
arp[17] = 0x0060dd440b75     # mark6-4031 eth3
arp[33] = 0x0060dd444618     # mark6-4031 eth4
arp[49] = 0x0060dd444619     # mark6-4031 eth5
print 'Using ARP table : %s' % (str(arp))

# Src and Dest IP
ip_base = [10,10,1,1]
for ip_part, name in ((1,'tengbe_0'), (17,'tengbe_1')):

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
# calculate reference epoch
ref_ep_num = 30 # 2014 part 2 = 29, 2015 part 1 = 30
ref_ep_date = datetime(2015,1,1,0,0,0) # date of start of epoch January 1 2015
print 'Using VDIF reference epoch %d (%s)' % (ref_ep_num, str(ref_ep_date))
print 'Synchronizing R2DBE to computer NTP time...'

##############
#   W0
##############
utcnow = datetime.utcnow()

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

roach2.write_int('r2dbe_vdif_0_hdr_w3_station_id', station_id_0)
roach2.write_int('r2dbe_vdif_1_hdr_w3_station_id', station_id_1)


############
#   W4
############

# nothing to do

############
#   W5
############

# nothing to do

############
#   W6
############

# nothing to do

############
#   W7
############

# nothing to do


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
roach2.write_int('r2dbe_vdif_1_enable', 0)  # disable for now to avoid confusion
