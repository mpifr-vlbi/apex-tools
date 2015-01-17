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
	'''Waits until UTC datetime T, corrected for local clock offset (e.g. TAI time)'''
	global offsetUTC
	iter = 0
	while True:
		Tcurr = datetime.datetime.utcnow()
		dT = T - Tcurr
		dT = dT.total_seconds() - offsetUTC
		if (dT <= 0):
			break
		# print dT
		iter = iter + 1
		time.sleep(0.1)
	# print 'waitUntil %s / now=%s : dT=%s iter=%d' % (T,Tcurr,str(dT),iter)
	if (iter > 0):
		return True	# Time reached by waiting
	else:
		return False	# Time was already past

def datetime2SNP(t):
        # Example SNP timestamp: 2015.016.07:30:00
        return t.strftime('%Y.%j.%H:%M:%S')

def SNP2datetime(tsnp):
        # Example SNP timestamp: 2015.016.07:30:00
        return datetime.datetime.strptime(tsnp, '%Y.%j.%H:%M:%S')


def splitLine(l):
	'''Splits .obs file line with date/time, duration, and command into respective pieces'''
	l = l.strip()
	while (l.find('  ') >= 0):
		l = l.replace('  ', ' ')
	i1 = l.find(' ')
	i2 = l.find(' ', i1+1)
	tstart = l[:i1]
	tdur   = l[(i1+1):i2]
	cmd    = l[(i2+1):]
	return [tstart,tdur,cmd]


def execCommand(cmd):
	# TODO: log
	global offsetUTC, ntpserver
	try:
		eval(cmd, globals(), locals())
		return True
	except:
		print 'Command %s failed with : %s' % (cmd,sys.exc_info()[0])
		return False


def handleTask(t):
	'''Task is a list of [tstart,tdur,cmd] as returned by splitLine() for one line on the .obs file'''
	if (len(t) != 3):
		print 'Invalid task %s' % (str(t))
		return None

	cmd = t[2]

	if (t[0] == '@always'):
		execCommand(cmd)
		return True

	Tstart = SNP2datetime(t[0]) 
	reached = waitUntil(Tstart)
	if not(reached):
		print 'Time %s for command %s already passed. Skipping.' % (t[0],cmd)
		return False

	execCommand(cmd)
	return True


def run(args):
	global ntpserver, offsetUTC

	if (len(args) != 2):
		usage()
		sys.exit(-1)
	waitUntil(datetime.datetime.utcnow())

	offsetUTC = utilsNTP.get_UTC_offset(ntpserver)	
	offsetUTC = int(offsetUTC)
	print 'Using UTC-TAI offset of %+d seconds' % (offsetUTC)

	fd = open(args[1], 'r')
	for line in fd:
		line = line.strip()
		if (len(line)<1) or (line[0]=='#'):
			continue
		task = splitLine(line)
		handleTask(task)
	fd.close()

run(sys.argv)
