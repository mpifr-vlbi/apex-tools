#!/usr/bin/python
"""
Usage: timing_query.py [<r2dbe hostname>]

Check how many frequency reference clock ticks
are counted between every external 1PPS. This
allows to check for clock drift.
"""

import adc5g, corr
from time import sleep, time
import sys, socket

r2dbe_hostname = 'r2dbe-1'
if len(sys.argv)==2:
    r2dbe_hostname = sys.argv[1]

# Check if we can resolve the host
hip = ''
try:
    hip = socket.gethostbyname(r2dbe_hostname)
except:
    pass
if len(hip) < 4:
    print('Error: could not get host %s' % (r2dbe_hostname))
    sys.exit(0)

# Connect and check timing
roach2 = corr.katcp_wrapper.FpgaClient(r2dbe_hostname)
roach2.wait_connected()
while True:
	t = int(time())
	r = roach2.read_int('r2dbe_onepps_gps_pps_per') 
	print '%d %d' % (t,r)
	sleep(1)
