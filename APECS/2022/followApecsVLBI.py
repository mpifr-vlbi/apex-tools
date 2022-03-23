#!/usr/bin/env python

import sys
import datetime
import time
import signal
import platform
import curses

def usage():
	print ('')
	print ('DEBUG / Usage: followApecsVLBI.py <experiment.obs>')
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

def utc_now():
	'''
	Return current UTC.
	On observer3 that runs on TAI, account for TAI leading UTC by some nr of leap seconds.
	'''
	T = datetime.datetime.utcnow()
	T += datetime.timedelta(seconds=-offsetUTC)
	return T


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


def showTaskList(stdscr, currtaskidx):
	'''
	Show the series of tasks (list[str]) on the screen, highlighting
	a "current" task at given index and placing it at the center row
	of the screen.
	'''
	global taskQueue, taskState

	stdscr.clrtobot()
	stdscr.nodelay(True)
	# curses.cbreak()

	# Dimensions of screen, and start-stop indices of tasks to display
	Ntasks = len(taskQueue)
	Nscreenrows = curses.LINES
	startrow = max(0, Nscreenrows//2 - currtaskidx)
	starttask = max(0, currtaskidx - Nscreenrows//2)
	endtask = min(Ntasks, starttask + (Nscreenrows-startrow) - 2)

	# Show tasks and their timing and status
	tcurr = utc_now()
	for n in range(starttask, endtask):
		task = taskQueue[n]
		taskstate = taskState[n]

		msg = '%-12s' % (taskState2Str(taskState[n]))
		msg += task['tstart_snp'] + '--' + task['tend_snp']

		if n == currtaskidx:
			rowattrib = curses.A_STANDOUT
		else:
			rowattrib = 0

		if taskstate == STATE_PENDING and n == currtaskidx:
			msg += ' (in %d seconds)' % ((task['tstart'] - tcurr).total_seconds())
		elif taskstate == STATE_ACTIVE:
			msg += ' (%d seconds remaining)' % ((task['tend'] - tcurr).total_seconds())
		msg += ' ' + task['cmd']

		msg = msg[0:(curses.COLS-5)]  # crop to less than screen width

		row = startrow + n - starttask
		if len(msg) > 0:
			stdscr.addstr(row, 0, msg, rowattrib)
			stdscr.clrtoeol()

	stdscr.refresh()


def loadApecsScript(obsfile):
	'''
	Gather all timed tasks in the schedule/script into an internal list.
	'''
	global taskQueue, taskState
	taskQueue = []
	taskState = []

	fd = open(obsfile, 'r')

	talways = utc_now()  # when the first/next @always should start

	for line in fd:

		line = line.strip()
		if (len(line)<1) or (line[0]=='#'):
			continue

		task = splitLine(line)  # should have: [tstart,tdur,cmd]
		if (len(task) != 3):
			print ('Invalid task %s' % (str(t)))
			continue

		if '@always' in task[0]:
			tstart = talways
			dur = int(task[1])
			cmd = '<<@always>> ' + str(task[2])
			talways += datetime.timedelta(seconds=dur+2)
		else:
			tstart = SNP2datetime(task[0])
			dur = int(task[1])
			cmd = str(task[2])

		tend = tstart + datetime.timedelta(seconds=dur)
		tstart_snp = datetime2SNP(tstart)
		tend_snp = datetime2SNP(tend)

		entry = {'tstart':tstart, 'tstart_snp':tstart_snp, 'tend':tend, 'tend_snp': tend_snp, 'dur':dur, 'cmd':cmd}
		taskQueue.append(entry)
		taskState.append(STATE_PENDING)

	fd.close()


def followApecsVLBI(stdscr, obsfile):
	global taskQueue, taskState

	# Work through the list of tasks
	currTaskIdx = 0
	while currTaskIdx < len(taskQueue):

		if gotCtrlC:
			return False

		# Pick active task
		task = taskQueue[currTaskIdx]

		# Pending, ongoing, completed/missed?
		tcurr = utc_now()
		if tcurr < task['tstart']:
			taskState[currTaskIdx] = STATE_PENDING
		elif tcurr >= task['tstart'] and tcurr <= task['tend']:
			taskState[currTaskIdx] = STATE_ACTIVE
		elif tcurr > task['tend']:
			if taskState[currTaskIdx] != STATE_ACTIVE:
				taskState[currTaskIdx] = STATE_MISSED
				currTaskIdx += 1
				continue  # skip a screen refresh and a sleep()
			else:
				taskState[currTaskIdx] = STATE_OVER
				currTaskIdx += 1

		# Display list of tasks on screen centered on current task
		showTaskList(stdscr, currTaskIdx)

		time.sleep(0.25)


#########################################################################################
#### Main
#########################################################################################

def run(cmdfile):

	signal.signal(signal.SIGINT, signal_handler)

	stdscr = curses.initscr()

	loadApecsScript(cmdfile)
	followApecsVLBI(stdscr, cmdfile)

	curses.nocbreak()
	stdscr.keypad(0)
	curses.echo()
	curses.endwin()


if (len(sys.argv) != 2):
	usage()
	sys.exit(-1)
else:
	run(sys.argv[1])

