#!/usr/bin/env python

import sys
import datetime
import time
import signal
import socket

def usage():
	print ('')
	print ('Usage: apecsVLBI.py <experiment.obs>')
	print ('')
	print ('The .obs file can be generated from a VEX file using vex2apecs.py,')
	print ('or from the vex2xml output XML file using xml2apecs.py.')
	print ('The .obs file has to contain three columns, the starting UT time,')
	print ('duration and the APECS command to execute at the given time.')
	print ('Note that APECS must be in remote_control(\'on\') mode.')
	print ('')

## Globals

logfile = None

# APECS system (observer3) runs from abm.apex-telesc .IRIG. that is running TAI
# The TAI time leads UTC by 37 seconds as of April 2018
# http://www.leapsecond.com/java/gpsclock.htm
#import utilsNTP
#ntpserver = 'time.nist.gov' #'nist1-lnk.binary.net'
offsetUTC = 37

# Ctrl-C
gotCtrlC = False

# APECS remote mode
use_remote_mode = True
APECS_host = "10.0.2.170"   # IP of observer3.apex-telescope.org
APECS_port = 22122          # UDP port on which APECS accepts commands (APECS: remote_control('on'))

## Functions

def signal_handler(signal, frame):
	global gotCtrlC
	gotCtrlC = True
	print('Ctrl-C pressed, stopping...')

def waitUntil(T,Tsnp='',msg=''):
	'''Waits until UTC datetime T, corrected for local clock offset (e.g. TAI time)'''
	global offsetUTC, gotCtrlC
	iter = 0
	while True:
		Tcurr = datetime.datetime.utcnow() + datetime.timedelta(seconds=-offsetUTC)
		dT = T - Tcurr
		dT = dT.total_seconds()
		if (dT <= 0) or gotCtrlC:
			break
		# print (dT)
		iter = iter + 1
		sys.stdout.write('\r')
		sys.stdout.write('Still %ds from now (%s) to start (%s) of %s' % (int(dT),datetime2SNP(Tcurr),Tsnp,msg))
		sys.stdout.flush()
		time.sleep(0.25)

	# print ('waitUntil %s / now=%s : dT=%s iter=%d' % (T,Tcurr,str(dT),iter))
	if gotCtrlC:
		return False	# Cancelled
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
	global APECS_host, APECS_port, use_remote_mode

	writeLog(cmd)

	if (cmd.find('interactive') > 0):
		print ('>>> VLBI execution paused, without timeout on pause, to allow')
		print ('>>> user commands entered into APECS. Be careful not to exceed')
		print ('>>> scan gap time. When done in APECS type \'cont\' to continue here.')
		while True:
			l = raw_input('type \'cont\' to continue VLBI> ')
			if ('cont' in l.lower().strip()):
				break	
		return True

	if not(use_remote_mode):
		# Local mode, apecsVLBI.py should be invoked from within APECS
		try:
			eval(cmd, globals(), locals())
			return True
		except:
			print ('Command %s failed with : %s' % (cmd,sys.exc_info()[0]))
			return False
	else:
		# Remote mode, commands are sent to APECS over UDP (requires remote_control('on')) in APECS)
		sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		if sys.version_info[0] >= 3:
			# sock.sendto(bytes(cmd,'utf-8'), (APECS_host, APECS_port)) # Python 3.x
			sock.sendto(bytes(cmd,'ascii'), (APECS_host, APECS_port)) # Python 3.x
		else:
			sock.sendto(cmd, (APECS_host, APECS_port)) # Python 2.x
		sock.close()

def writeLog(s):
	global logfile
	T = datetime.datetime.utcnow() + datetime.timedelta(seconds=-offsetUTC)
	T = datetime2SNP(T)
	info = '%s;\"%s' % (T,s)
	logfile.write(info + '\n')
	print (info)

def handleTask(t):
	'''Task is a list of [tstart,tdur,cmd] as returned by splitLine() for one line on the .obs file'''
	global gotCtrlC

	if (len(t) != 3):
		print ('Invalid task %s' % (str(t)))
		return None

	cmd = t[2]

	if (t[0] == '@always'):
		execCommand(cmd)
		time.sleep(2)
		return True

	Tstart = SNP2datetime(t[0]) 
	reached = waitUntil(Tstart, Tsnp=t[0],msg=cmd)
	if gotCtrlC:
		return False
	if not(reached):
		print ('Time %s for command %s already passed. Skipping.' % (t[0],cmd))
		return False

	execCommand(cmd)
	return True


def apecsVLBI(obsfile):
	global offsetUTC, gotCtrlC, logfile
	# global ntpserver

	logfile = open(obsfile + '.log', 'a')

	# offsetUTC = utilsNTP.get_UTC_offset(ntpserver)	
	offsetUTC = int(offsetUTC)

	writeLog('----------| START OF %s |----------' % (obsfile))
	writeLog('Using TAI-UTC offset of %+d seconds' % (offsetUTC))

	if False: # test
		T0 = datetime.datetime.utcnow()
		waitUntil(T0,datetime2SNP(T0),'test')

	fd = open(obsfile, 'r')
	for line in fd:
		line = line.strip()
		if (len(line)<1) or (line[0]=='#'):
			writeLog(line)
			continue
		task = splitLine(line)
		handleTask(task)
		if gotCtrlC:
			writeLog('----------| USER-REQUESTED STOP OF  %s |----------' % (obsfile))
			break
	fd.close()
	logfile.close()

signal.signal(signal.SIGINT, signal_handler)

if (len(sys.argv) != 2):
	usage()
	sys.exit(-1)
else:
	apecsVLBI(sys.argv[1])

