#!/usr/bin/env python
# e.g. /alma/ACS-2020JUN/pyenv/versions/2.7.16/bin/python
'''
Collect weather data from APECS databases for the time range
of a specified VEX file - determined by VEX $SCHED section
fields exper_nominal_start and exper_nominal_stop.

The weather data are outputted in a format very similar
to the FieldSystem wx format.

  wx/temp,press,humid,speed,direction,pwv
   where
     temp          temperature, degrees C.
     press         barometric pressure, mbars.
     humid         humidity, %.
     speed         wind speed, meters/second
     direction     wind direction, degrees of azimuth
     pwv           precip. water vapor, mm, note: not a standard WX field
'''

from apps.db import DBQuery
from types import *
from support.timestamp import timestamp
from string import replace, join

import os, re, time, sys
import argparse
import datetime

import numpy as np
from scipy.interpolate import Akima1DInterpolator

__author__ = "Jan Wagner (MPIfR)"
__version__ = "1.0.0"


def parse_args(args):
	cmd = '%(prog)s <options>'
        parser = argparse.ArgumentParser(description=__doc__, add_help=True, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument('--version', action='version', version='%(prog)s ' + __version__)
	parser.add_argument('-t', dest='tstart', default='', help='start time as vex-format timestamp')
	parser.add_argument('-T', dest='tstop', default='', help='stop time as vex-format timestamp')
	parser.add_argument('-i', dest='interval', default=60, help='interval in seconds to to data points (default: %(default)d)')
	parser.add_argument('vexfile', nargs='?')

	return parser.parse_args(args)


def SNP2datetime(tsnp):
	'''Convert an SNP timestamp string (like 2015.016.07:30:00) into Python datetime'''
	return datetime.datetime.strptime(tsnp, '%Y.%j.%H:%M:%S')


def vex2datetime(tvex):
	'''Convert a VEX timestamp string (like 2015y016d07h30m00s) into Python datetime'''
	return datetime.datetime.strptime(tvex, '%Yy%jd%Hh%Mm%Ss')


def datetime2SNP(dt):
	'''Convert datetime into a SNP timestamp like 2015.016.07:30:00'''
	return dt.strftime('%Y.%j.%H:%M:%S')


def median_abs_deviation(data, axis=None):
	'''Return Median Absolute Deviation of numpy array'''
	return np.median(np.abs(data - np.median(data, axis)), axis)


def wrap_picosec_value(v):
	lmt = 500000000
	if v >= lmt:
		v = 2*lmt - v
	return v


#----------------------------------------------------------------------

def extractVexTimerange(vexname):
	'''Open given VEX file and extract nominal start and stop time strings'''

	tstart = None
	tstop = None

	with open(vexname, 'r') as f:
		for line in f:
			line = line.strip()
			if len(line) <= 1 or line[0] == '*':
				continue
			if "exper_nominal_" not in line:
				continue
			# todo: regexp?
			line = replace(line, ';', ' ')
			elems = line.split('=')
			if elems[0] == "exper_nominal_start":
				tstart = str(elems[1]).strip()
			elif elems[0] == "exper_nominal_stop":
				tstop = str(elems[1]).strip()
			# Got both?
			if tstart and tstop:
				break
	return (tstart, tstop)

#----------------------------------------------------------------------

class APECSQuery:
	'''Based on MonitorQuery'''

	def __init__(self, key):
		self.key = key

	def get(self, start=1618268400, stop=1618271940):
		map_ids, things = {self.key: 1}, (self.key,)
		return self.__get_data(map_ids, start, stop, things)

	def loadAndInterpolate(self, start, stop, tinterp):
		raw = self.get(start, stop)
		if len(raw) > 0:
			self.times = np.array([int(item[0]) for item in raw])
			self.vals = np.array([float(item[1]) for item in raw])
			self.ivals = Akima1DInterpolator(self.times, self.vals)(tinterp)
			return (self.times, self.vals, self.ivals)
		return ([],[],[])

	def __get_data(self,map_ids,start_time,stop_time,things):

		rowCount = 0
		mydb = DBQuery.DBQuery()
		rawresult = []

		for table in things:

			query = "select ts,value from `%s` where ts>'%s' and ts<='%s' order by ts asc" % \
				(table, timestamp(start_time,fmt='ODBC',TZ=None),
				timestamp(stop_time,fmt='ODBC',TZ=None))

			mydb.db.sql(query)
			stuff = mydb.db.fetchall()
			if stuff is None or len(stuff) == 0:
				continue

			#rawresult = []
			for (time_now, v) in stuff:
				now = self.__string1_to_utc(str(time_now))
				rawresult.append([now, v])
			if len(stuff) > rowCount:
				rowCount  = len(stuff)
				time_end,v = stuff[-1]
			if stuff is None or len(stuff) == 0:
				return

		return rawresult


	def __string1_to_utc(self, ts_string):
		'''Return the time from a UT string.'''
		now = int (time.mktime(time.strptime(ts_string[0:19],'%Y-%m-%d %H:%M:%S')))
		t = time.localtime(now)
		if t[8] == 0:
			now = now - time.timezone
		elif t[8] == 1:
			now = now - time.altzone
		return now


#----------------------------------------------------------------------

if __name__ == "__main__":

	opts = parse_args(sys.argv[1:])

	if opts.vexfile:
		tstart, tstop = extractVexTimerange(opts.vexfile)
		vexname = os.path.basename(opts.vexfile)
		vexname = vexname.split('.')[0] # could be tidier...
	else:
		tstart = opts.tstart
		tstop = opts.tstop
		vexname = 'unspecifiedVex'

	if not (tstart and tstop):
		print("No time range specified or found in VEX.")
		print("Currently got tstart of '%s', tstop of '%s'" % (str(tstart),str(tstop)))
		sys.exit(0)

	Unix0 = datetime.datetime(1970, 1, 1)
	tstart_dt = vex2datetime(tstart)
	tstop_dt = vex2datetime(tstop)
	tstart_unix = (tstart_dt - Unix0).total_seconds()
	tstop_unix = (tstop_dt - Unix0).total_seconds()
	# tstop_unix = tstart_unix + 1240 # DEBUG
	output_times = np.arange(tstart_unix, tstop_unix, int(opts.interval))

	print("Looking up data between '%s' (Unix %d) and '%s' (Unix %d)" % (str(tstart),tstart_unix,str(tstop),tstop_unix))

	# Query APECS databases
        counterFmout = APECSQuery('APEX:COUNTERS:GPSMINUSFMOUT:GPSMINUSFMOUT')
        counterMaser = APECSQuery('APEX:COUNTERS:GPSMINUSMASER:GPSMINUSMASER')
	counterFmout.loadAndInterpolate(tstart_unix, tstop_unix, output_times)
	counterMaser.loadAndInterpolate(tstart_unix, tstop_unix, output_times)

	if len(counterFmout.times) <= 0:
		print('No data found in the given time range')
		sys.exit(0)

	# Output GPS Offset data
	clkfilename = vexname + 'ax.clock.log'
	with open(clkfilename, 'w') as clkfile:
		print("Writing GPS-FMOUT and GPS-Maser data into '%s'" % (clkfilename))
		t = datetime2SNP(Unix0 + datetime.timedelta(seconds=output_times[0]))
		clkfile.write('%s;APEX Database GPS vs Devices offset for %s\n' % (t, vexname))
		clkfile.write('%s;The format is gps-fmout/[seconds] and gps-maser/[seconds]\n' % (t))
		for n in range(1,len(output_times)):
			t = datetime2SNP(Unix0 + datetime.timedelta(seconds=output_times[n]))
			diffF = wrap_picosec_value(counterFmout.ivals[n]) * 1e-9
			diffM = wrap_picosec_value(counterMaser.ivals[n]) * 1e-9
			#clkStr = '%s/clock/gps-fmout=%.9f/gps-maser=%.9f\n' % (t, diffF, diffM)
			#clkfile.write(clkStr)
			clkStr = '%s/gps-fmout/%+.6e\n' % (t, diffF)
			clkfile.write(clkStr)
			clkStr = '%s/gps-maser/%+.6e\n' % (t, diffM)
			clkfile.write(clkStr)
