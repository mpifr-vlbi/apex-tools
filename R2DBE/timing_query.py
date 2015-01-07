#!/usr/bin/python
#
# Check how many frequency reference clock ticks
# are counted between every external 1PPS. This
# allows to check for clock drift.
#

import adc5g, corr
from time import sleep, time

roach2 = corr.katcp_wrapper.FpgaClient('r2dbe-1')
roach2.wait_connected()

while True:
	t = int(time())
	r = roach2.read_int('r2dbe_onepps_gps_pps_per') 
	print '%d %d' % (t,r)
	sleep(1)
