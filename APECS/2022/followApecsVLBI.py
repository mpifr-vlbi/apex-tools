#!/usr/bin/env python

import sys
import datetime
import time
import signal
import platform
import curses

def usage():
	print ('')
	print ('Usage: followApecsVLBI.py <experiment.obs>')
	print ('')
	print ('The .obs file can be generated from a VEX file using vex2apecs.py,')
	print ('or from the vex2xml output XML file using xml2apecs.py.')
	print ('Unlike apecsVLBI.py that actually sends commands to APECS,')
	print ('this script merely *displays* the progress of the timed commands')
	print ('in the script without sending anything (someday this display')
	print ('will be integrated into apecsVLBI.py itself...)')

###########################################################################################################
#### Globals
###########################################################################################################

# APECS system (observer3) runs from abm.apex-telesc .IRIG. that is running TAI
# VLBI is using UTC; need to correct for http://www.leapsecond.com/java/gpsclock.htm
if ('observer3' not in platform.node()) and ('10.0.2.170' not in platform.node()):
	offsetUTC = 0
	print ('\nINFO: Apparently not running on Observer3. Not applying TAI/UTC leap seconds correction!\n')
else:
	# The TAI time leads UTC by 37 seconds as of February 2020, likely to change June/July 2020
	offsetUTC = 37
	print ('\nINFO: Apparently running on Observer3. Correction computer time (TAI) by %d leap seconds to have UTC!\n' % (offsetUTC))

# Ctrl-C
gotCtrlC = False

# Tasks list history and at some point also future
taskQueue=['']
taskState=[0]
STATE_MISSED, STATE_PENDING, STATE_ACTIVE, STATE_OVER = -1,0,1,2

###########################################################################################################
#### Functions
###########################################################################################################

def signal_handler(signal, frame):
	global gotCtrlC
	gotCtrlC = True
	print('Ctrl-C pressed, stopping...')

def waitUntil(stdscr,T,Tsnp='',msg=''):
	'''Waits until UTC datetime T, corrected for local clock offset (e.g. TAI time)'''
	global offsetUTC, gotCtrlC
	iter = 0
	while True:
		Tcurr_obs3 = datetime.datetime.utcnow()
		Tcurr = Tcurr_obs3 + datetime.timedelta(seconds=-offsetUTC)
		dT = T - Tcurr
		dT = dT.total_seconds()

		# skip the wait: for testing purposes:
		#dT = 0

		if (dT <= 0) or gotCtrlC:
			break		

		iter = iter + 1
		info = 'Still %ds from now (%s) to start (%s) of %s' % (int(dT),datetime2SNP(Tcurr),Tsnp,msg)
		stdscr.addstr(curses.LINES//2, 0, info)
		stdscr.clrtoeol()
		stdscr.refresh()
		time.sleep(0.25)

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

def taskState2Str(state):
	if state == STATE_MISSED:
		return 'MISSED'
	if state == STATE_PENDING:
		return 'PENDING'
	if state == STATE_ACTIVE:
		return 'ACTIVE'
	if state == STATE_OVER:
		return 'OVER'
	return 'UnkwState'

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


def showTaskList(stdscr, tasks, currtaskidx, taskstatuses=None):
	'''
	Show the series of tasks (list[str]) on the screen, highlighting
	a "current" task at given index and placing it at the center row
	of the screen.

	If 'taskstatuses' is given, each index in 'tasks' must have at
	the same index an entry in list 'taskstatuses'. The expected
	values are -1=missed, 0=pending, 1=ongoing, 2=completed or errored out.
	'''

	stdscr.clrtobot()
	stdscr.nodelay(True)
	curses.cbreak()

	Ntasks = len(tasks)
	Nscreenrows = curses.LINES
	startrow = max(0, Nscreenrows//2 - currtaskidx)
	starttask = max(0, currtaskidx - Nscreenrows//2)
	endtask = min(Ntasks, starttask + (Nscreenrows-startrow) - 2)

	for n in range(starttask, endtask):
		Tsnp, Tdur, cmd = tasks[n][0:3]

		msg = ''
		if taskstatuses:
			statestr = taskState2Str(taskstatuses[n])		
			msg = '%-12s' % (statestr)
		msg += Tsnp + ' ' + cmd
		msg = msg[0:(curses.COLS-5)]

		row = startrow + n - starttask
		if len(msg) > 0:
			stdscr.addstr(row, 0, msg)
			stdscr.clrtoeol()

	stdscr.refresh()


def followApecsVLBI(stdscr, obsfile):
	global taskQueue, taskState

	taskQueue = []
	taskState = []

	# Grab full list of all timed tasks in the schedule
	fd = open(obsfile, 'r')
	for line in fd:
		line = line.strip()
		if (len(line)<1) or (line[0]=='#'):
			continue
		task = splitLine(line)  # should have: [tstart,tdur,cmd]
		if (len(task) != 3):
			print ('Invalid task %s' % (str(t)))
		else:
			taskQueue.append(task)
			taskState.append(STATE_PENDING)
	fd.close()

	# Work through the list of tasks
	currTaskIdx = 0
	for currTaskIdx in range(len(taskQueue)):

		if gotCtrlC:
			return False

		task = taskQueue[currTaskIdx]
		taskState[currTaskIdx] = STATE_ACTIVE
		if '@always' in task[0]:
			taskState[currTaskIdx] = STATE_ACTIVE
		else:
			Tsnp, Tdur, cmd = task[0:3]
			Tstart = SNP2datetime(Tsnp)
			reached = waitUntil(stdscr, Tstart, Tsnp=Tsnp, msg=cmd)
			if reached:
				taskState[currTaskIdx] = STATE_ACTIVE
			else:
				taskState[currTaskIdx] = STATE_MISSED

		if currTaskIdx>0 and taskState[currTaskIdx-1] != STATE_MISSED:
			taskState[currTaskIdx-1] = STATE_OVER

		showTaskList(stdscr, taskQueue, currTaskIdx, taskState)

#########################################################################################
#### ENTRY
#########################################################################################

def run(cmdfile):
	signal.signal(signal.SIGINT, signal_handler)
	stdscr = curses.initscr()
	followApecsVLBI(stdscr, cmdfile)

if (len(sys.argv) != 2):
	usage()
	sys.exit(-1)
else:
	run(sys.argv[1])

