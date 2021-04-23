#!/bin/python
import os, sys
import glob
import csv
import re

from datetime import datetime

import BoaMBFits

# RAW_DATA_ROOT = '/homes/t-0107.f-9996a-2021/rawdata-copy-APEX-2021-04-09/'  # where to look for scans
RAW_DATA_ROOT = '/homes/t-0107.f-9996a-2021/rawdata/'

WOB_POSITION_MARGIN = 5.0  # positions by amount of margin within throw are considered as settled ON(/OFF)-source

TYPE_KEY = 'WOB'        # filter for CSV data, substring that the TYPE_COL must contain
LINE_KEY = 'vlbifreq'   # filter for CSV data, substring that the LINE_COL must contain

TIME_COL = 0  # CSV file column with time YYYY-MM-DDTHH:MM:SS
SCAN_COL = 1  # CSV file column with scan <nnnn>
LINE_COL = 5  # CSV file column with line freq name e.g. 'vlbifreq'
TYPE_COL = 9  # CSV file column with switching mode command, e.g., 'TP()' or 'WOB(<throw>,<dur>)'

def usage():

	print('')
	print('Usage: %s <scans.csv>' % (sys.argv[0]))
	print('')
	print('where the scans file is structured like')
	print('   YYYY-MM-DDTHH:MM:SS,Scan,SourceName,ScanType,Frequency,Line,Az,El,Duration,SwitchingMode,PWV,Temp,Hum,Wind')
	print('   2021-04-01T22:45:23,6330,WB947,on,230.538,12CO(2-1)U,+7.3,+79.3,121,TP,1.55,-2.4,31.6,6.2')
	print('   2021-04-03T23:29:30,6916,J0522-3627,on,227.103,vlbifreq7,-121.6,+56.0,58,"WOB(80.0,1.0)",2.80,-0.8,33.0,3.')
	print('')


def filterCSV(csvfilename):

	scans = []

	with open(csvfilename, 'r') as csvfile:

		input = csv.reader(csvfile, delimiter=',')
		next(input, None) # skip the CSV header

		for row in input:

			if len(row) <= TYPE_COL:
				continue

			time, scan, linename, type = row[TIME_COL], row[SCAN_COL], row[LINE_COL], row[TYPE_COL]
			if TYPE_KEY not in type or LINE_KEY not in linename:
				continue

			scan = int(scan)
			wob_cmd_fields = re.split('\(|,|\)', type) # try to tokenize 'WOB(80.0,1.0)' => 'WOB','80.0','1.0'
			wob_throw = float(wob_cmd_fields[1])

			filepattern = RAW_DATA_ROOT + '/APEX-%d-*/' % (scan)
			datasetdir = glob.glob(filepattern)
			if len(datasetdir) > 0:
				scans.append( (datasetdir[0], 1, wob_throw) )
			else:
				print('Skipping scan with no dataset directory: ',time,scan,linename,type, filepattern)

	return scans


def extractOffSourceTimes(datasetdir, subscan, wob_throw_asec, fake=False):
	'''
	Inspect a wobbler on() dataset.
	Returns a list of [tstart,tstop] Unix timestamp pairs with OFF-source and wobbler transitioning time ranges.
	'''

	flagged_unix_timeranges = []
	if fake:
		return [[1617931071.5159998, 1617931071.9799998], [1617931072.452, 1617931072.98], [1617931073.4520001, 1617931073.9800003], [1617931074.4480002, 1617931074.9799998]]

	ds = BoaMBFits.importDataset(datasetdir)

	scanTables = ds.getTables(EXTNAME='SCAN-MBFITS')
	# print(scanTables)
	scanTable = scanTables[0]
	tai2utc = scanTable.getKeyword('TAI2UTC').getValue()

	monTables = ds.getTables(EXTNAME='MONITOR-MBFITS', SUBSNUM=subscan)
	if type(monTables) is list:
		#print(monTables, type(monTables))
		monTable = monTables[0]
	else:
		monTable = monTables
	monTable.open()
	monTable.setSelection('MONPOINT=="WOBDISPL"')
	wobTime = (monTable.getColumn('MJD').read() - 40587.0) * 86400.0 - tai2utc
	wobPos = monTable.getColumn('MONVALUE').read()
	monTable.close()

	scanTables = ds.getTables(EXTNAME='SCAN-MBFITS') # , SUBSNUM=subscan)
	if type(scanTables) is list:
		scanTable = scanTables[0]
	else:
		scanTable = scanTables
	scanTable.open()
	# for keywd in ['PHASE1', 'PHASE2', 'WOBTHROW', 'WOBDIR', 'WOBCYCLE', 'WOBMODE', 'WOBPATT']:
	#	print(keywd + ': ', scanTable.getKeyword(keywd).getValue())
	# ('PHASE1: ', 'WOFF')
	# ('PHASE2: ', 'WON')
	# ('WOBTHROW: ', 0.0222222222222222)
	# ('WOBDIR: ', 'ALON')
	# ('WOBCYCLE: ', 1.0)
	# ('WOBMODE: ', 'SQUARE')
	# ('WOBPATT: ', 'POS')
	phase1 = scanTable.getKeyword('PHASE1').getValue()
	phase2 = scanTable.getKeyword('PHASE2').getValue()
	scanTable.close()

	ds.close()

	# Determine the wobbler position that is ON-source (e.g., is it -80.0, or is it +80.0)
	# Note: "at APEX the phase 1 wobbler position is always positive and the phase 2 one is negative"
	if phase1=='WOFF' and phase2=='WON':
		wob_onsource_asec = -abs(wob_throw_asec)
	elif phase1=='WON' and phase2=='WOFF':
		wob_onsource_asec = +abs(wob_throw_asec)
	else:
		print('Error: unexpected PHASE1-PHASE2 of %s-%s, expecting WOFF-WON or WON-WOFF!' % (phase1,phase2))
		return []

	count_ons = 0
	count_offs = 0
	t_off_start = 0
	t_off_end = 0

	for i in range(len(wobTime)):

		unix_time = float(wobTime[i])
		wob_phase = wobPos[i][0]*3600.

		isOnSource = abs(wob_phase - wob_onsource_asec) < WOB_POSITION_MARGIN
		if isOnSource:
			count_ons += 1
			# If we got here after an OFF-source phase completed, write out the flagging for that phase
			if t_off_start > 0:
				flagged_unix_timeranges.append( [t_off_start, t_off_end] )
				t_off_start = 0
				t_off_end = 0
		else:
			count_offs += 1
			if t_off_start <= 0:
				t_off_start = unix_time
			t_off_end = unix_time

		# dtime = datetime.utcfromtimestamp(unix_time)
		# print ('%s %.2f : %s source' % (dtime.strftime('%Y-%m-%d %H:%M:%S.%f'), wob_phase, posInfo))

	print('Dataset %s had %d wobbler entries, %d on-source and %d off-source' % (datasetdir, count_ons+count_offs, count_ons, count_offs))

	# print(flagged_unix_timeranges)
	return flagged_unix_timeranges


uvflag_header_written = 0

def generateUVFLAG(timeranges, outfile, heading='next apecs scan'):
	'''
	Argument 'timeranges' is a list of [tstart,tstop] pairs of Unix timestamps.
	Output is similar to

	opcode = 'FLAG'
	dtimrang = 1  timeoff = 0 
	ant_name='Ax' timerang= 19,20,11,59,  19,20,12,14  reason='Wobbler off source.' /
	ant_name='Ax' timerang= 19,20,16,59,  19,20,17,13  reason='Wobbler off source.' /
	...
	  with UT times in doy,hh,mm,ss format.
	'''
	global uvflag_header_written

	if uvflag_header_written == 0:
		outfile.write("opcode = 'FLAG'\n")
		outfile.write("dtimrang = 1  timeoff = 0\n")
		uvflag_header_written = 1

	outfile.write('! %s\n' % (heading))
	for timerange in timeranges:
		t0,t1 = timerange
		dtime0 = datetime.utcfromtimestamp(t0)
		dtime1 = datetime.utcfromtimestamp(t1)
		str0 = dtime0.strftime('%-j,%H,%M,%S.%f')
		str1 = dtime1.strftime('%-j,%H,%M,%S.%f')
		outfile.write("ant_name='Ax' timerang=%s,%s reason='Wobbler off source.' /\n" % (str0,str1))

	outfile.flush()
	os.fsync(outfile)


def processCSV(csvfilename, outfilename):

	scans = filterCSV(csvfilename)
	print('Found %d wobbler scans in the CSV file.' % (len(scans)))

	outfile = open(outfilename, "w")

	numflags = 0
	for scan in scans:
		datasetdir, subscan, throw_asec = scan[0],scan[1],scan[2]
		timeranges = extractOffSourceTimes(datasetdir, subscan, throw_asec)
		generateUVFLAG(timeranges, outfile, heading=datasetdir)
		numflags += len(timeranges)

	print('Wrote %d flag entries to %s' % (numflags, outfilename))


if len(sys.argv) != 2:
	usage()
else:
	processCSV(sys.argv[1], "apex.flags")

