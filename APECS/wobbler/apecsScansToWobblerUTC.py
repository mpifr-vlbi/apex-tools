#!/bin/python
import sys
import glob
import csv

import BoaMBFits

time_col = 0
scan_col = 1
freq_col = 5
type_col = 9  # column containing TP or WOB as a string

type_key = 'WOB'
freq_key = 'vlbifreq'

root = '/homes/t-0107.f-9996a-2021/rawdata-copy-APEX-2021-04-09/'
#root = '/homes/t-0107.f-9996a-2021/rawdata/'

def usage():

	print('')
	print('Usage: %s <scans.csv>' % (sys.argv[0]))
	print('')
	print('where the scans file is structured like')
	print('   YYYY-MM-DDTHH:MM:SS,Scan,SourceName,ScanType,Frequency,Line,Az,El,Duration,SwitchingMode,PWV,Temp,Hum,Wind')
	print('   2021-04-01T22:45:23,6330,WB947,on,230.538,12CO(2-1)U,+7.3,+79.3,121,TP,1.55,-2.4,31.6,6.2')
	print('   2021-04-03T23:29:30,6916,J0522-3627,on,227.103,vlbifreq7,-121.6,+56.0,58,"WOB(80.0,1.0)",2.80,-0.8,33.0,3.')
	print('')


def extractMBFITSTimes(dataset, subscan):

	ds = BoaMBFits.importDataset(dataset)

	scanTables = ds.getTables(EXTNAME='SCAN-MBFITS')
	print(scanTables)
	scanTable = scanTables[0]
	tai2utc = scanTable.getKeyword('TAI2UTC').getValue()

	monTables = ds.getTables(EXTNAME='MONITOR-MBFITS', SUBSNUM=subscan)
	if type(monTables) is list:
		print(monTables, type(monTables))
		monTable = monTables[0]
	else:
		monTable = monTables
	monTable.open()
	monTable.setSelection('MONPOINT=="WOBDISPL"')
	wobTime = (monTable.getColumn('MJD').read() - 40587.0) * 86400.0 - tai2utc
	wobPos = monTable.getColumn('MONVALUE').read()
	monTable.close()

	ds.close()

	for i in range(len(wobTime)):
		print ('%.3f %.2f' % (wobTime[i], wobPos[i][0]*3600.))


def parseCSV(csvfilename):

	with open(csvfilename, 'r') as csvfile:
		input = csv.reader(csvfile, delimiter=',')
		next(input, None) # skips header
		for row in input:
			if len(row) <= type_col:
				continue
			time, scan, tuning, type = row[time_col], row[scan_col], row[freq_col], row[type_col]
			if type_key in type and freq_key in tuning:
				scan = int(scan)
				filepattern = root + '/APEX-%d-*/' % (scan)
				fitsdir = glob.glob(filepattern)
				if len(fitsdir) > 0:
					print('Found fits dir for: ',time,scan,tuning,type, filepattern, fitsdir)
					#extractMBFITSTimes(fitsdir[0], 1)
				else:
					print('Skipped, no fits dir found for: ',time,scan,tuning,type, filepattern)

if len(sys.argv) != 2:
	usage()
	sys.exit(0)

parseCSV(sys.argv[1])
