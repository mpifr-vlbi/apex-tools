#!/usr/bin/env python

import sys
import datetime
import time

# APECS system runs from abm.apex-telesc .IRIG. with is running TAI
# The TAI-UTC offset is 35 seconds until June 2015
import utilsNTP
ntpserver = 'nist1-lnk.binary.net'
offsetUTC = 35

def usage():
	print ''
	print 'Usage: apecsVLBI.py <experiment.obs>'
	print ''
	print 'The .obs file can be generated from a VEX file using vex2apecs.py.'
	print 'It has to contain three columns, the starting UT time, duration and'
	print 'APECS command to execute at the given time.'
	print ''

def waitUntil(T):
	'''Waits until UTC datetime T'''
	while True:
		Tcurr = datetime.datetime.utcnow()
		dT = T - Tcurr
		dT = dT.total_seconds()
		if (dT <= 0):
			break
		# print dT
		time.sleep(0.1)

def run(args):
	global ntpserver, offsetUTC

	if (len(args) != 2):
		usage()
		sys.exit(-1)
	waitUntil(datetime.datetime.utcnow())

	offsetUTC = utilsNTP.get_UTC_offset(ntpserver)	
	offsetUTC = int(offsetUTC)
	print 'Using UTC-TAI offset of %+d seconds' % (offsetUTC)

run(sys.argv)
