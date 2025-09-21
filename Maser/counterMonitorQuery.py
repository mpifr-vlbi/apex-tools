#!/usr/bin/env python
# e.g. /alma/ACS-2020JUN/pyenv/versions/2.7.16/bin/python
#
# VLBI maserMonitorQuery derived from APEX MonitorQuery.
#
# Grabs raw data (nanosecond offsets) from the 
# APEX:COUNTERS:GPSMINUSFMOUT:GPSMINUSFMOUT or
# APEX:COUNTERS:GPSMINUSMASER:GPSMINUSMASER monitoring points.
#

from apps.db import DBQuery
import os, re, time, sys
import argparse

from types import *
from support.timestamp import timestamp
from string import replace, join
from time import gmtime
from copy import copy

import numpy as np

__author__ = "Jan Wagner (MPIfR)"
__version__ = "1.0.1"

def parse_args(args):
	cmd='%(prog)s <options>'
	
        parser = argparse.ArgumentParser(description=__doc__, add_help=True, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument('--version', action='version', version='%(prog)s ' + __version__)
	parser.add_argument('-d', '--days', dest='history_days', default='5.0', help='how many days of data to include %(default)s)')
	parser.add_argument('-t', dest='tstart', default='0', help='starting Unix second of time range to include (default: %(default)s)')
	parser.add_argument('-T', dest='tstop', default='0', help='last Unix second of time range to include (default: %(default)s)')
	parser.add_argument('-m', '--maser', action='store_true', help='get GPSMINUSMASER data')
	parser.add_argument('-f', '--fmout', action='store_true', help='get GPSMINUSFMOUT data')

	return parser.parse_args(args)


#----------------------------------------------------------------------
class APECSCounters:
	'''Based on MonitorQuery'''

	def __init__(self, key):
		self.key = key

	def get(self, start=1618268400, stop=1618271940):
		map_ids, things = {self.key: 1}, (self.key,)
		return self.__get_data(map_ids, start, stop, things)

	def __get_data(self,map_ids,start_time,stop_time,things):
		keep = {}
		rowCount = 0 
		mydb = DBQuery.DBQuery() 
		for table in things:
			query = "select ts,value from `%s` where ts>'%s' and ts<='%s' order by ts asc" % \
				(table, timestamp(start_time,fmt='ODBC',TZ=None),
			timestamp(stop_time,fmt='ODBC',TZ=None))
			mydb.db.sql(query)
			stuff = mydb.db.fetchall()
			self.data = []
			me=re.compile('(\d+)\.(\d+)\.(\d+)\.(\d+)')
			# make an array to put multi-columns into
			keep[table] = []

			if stuff is None or len(stuff) == 0:
				continue

			rawresult = []
			for (time_now, v) in stuff:
				now = self.__string1_to_utc(str(time_now))
				keep[table].append((now,str(v)))
				rawresult.append([now, v])
				# print('time:', now, 'data:', v)
           
			if len(stuff) > rowCount:
				rowCount  = len(stuff)
				time_end,v = stuff[-1]
			if stuff is None or len(stuff) == 0:
				return

		# make a 1 sec time grid
		time_now,v = stuff[0]
		#print('time_now',time_now,'time_end',time_end)
		start = self.__string1_to_utc(str(time_now))
		stop = self.__string1_to_utc(str(time_end))
		nrows = stop - start + 1  # difference in seconds
		ncols = len(things)
		rows = []   # hold column data
		# 
		# fill in table with NULLs to be later overwritten with
		# values
		#
		intern('NULL')
		# reuse blank array

		blank_arr = ['NULL' for i in range(ncols+1)]
		rows = [copy(blank_arr) for i in range(nrows)]
		for n in range(nrows):
			gmt = gmtime(start+n)
			hours = gmt[3]+gmt[4]/60.+gmt[5]/3600.
			rows[n][0]='%04d %02d %02d %f' % (gmt[0],gmt[1],gmt[2],hours)

		# now have array, now fill in data
		try:
			for id in things:
				col = map_ids[id]
				stuff = keep[id]
				for (now,v) in stuff:
					index = (now - start)
					rows[index][col] = v
		except IndexError:
			print ("get_data: Index error")

		# copy out
		ymdresult = []
		for n in range(nrows):
			# remove lines with all blanks
			if filter(lambda x: x != 'NULL', rows[n][1:]) != []:
				line = join(rows[n],' ')  # self.fd.write(join(rows[n],' ') + '\n')
				ymdresult.append(line)

		return (rawresult,ymdresult)

	def __string1_to_utc(self, ts_string):
		'''Return the time from a UT string.'''
		now = int (time.mktime(time.strptime(ts_string[0:19],'%Y-%m-%d %H:%M:%S')))
		t = time.localtime(now)
		if t[8] == 0:
			now = now - time.timezone
		elif t[8] == 1:
			now = now -time.altzone
		return now


def median_abs_deviation(data, axis=None):
    return np.median(np.abs(data - np.median(data, axis)), axis)


if __name__ == "__main__":

	opts = parse_args(sys.argv[1:])

	if opts.fmout:
		counter = APECSCounters('APEX:COUNTERS:GPSMINUSFMOUT:GPSMINUSFMOUT')
	elif opts.maser:
		counter = APECSCounters('APEX:COUNTERS:GPSMINUSMASER:GPSMINUSMASER')
	else:
		print('Specify either --maser or --fmout')
		sys.exit(1)	

	if int(opts.tstart) <= 0:
		current_unixtime = int(time.time())
		from_unixtime = current_unixtime - int(float(opts.history_days)*24*60*60)
		to_unixtime = current_unixtime

	# Query APECS databases
	rawdata, ymddata = counter.get(start=from_unixtime, stop=to_unixtime)

	# Dump negative (error?) values:
	for item in rawdata:
		if int(item[1]) < 0:
			print('Negative measurement, could be an error code: %d %s' % (item[0],item[1]))

	# Wrap the 0..1e9 nanosec values to +-0.5e9 nanosec
	times = np.array([int(item[0]) for item in rawdata])
	tdiff_ns = np.array([float(item[1]) for item in rawdata])
	flt_t, flt_c = [], []
	for t,c in zip(times,tdiff_ns):
		if c < 0:
			# discard error codes
			continue
		if c >= 0.9e9: #  TODO -- improve into an actual unwrap()
			c = c - 1.0e9
		flt_t.append(t)
		flt_c.append(c)
	times = np.array(flt_t)
	tdiff_ns = np.array(flt_c)  

	# Dump data into a file
	if True:
		fdump = open('counters.raw', 'w')
		fdump.write('# Time(unix second) Value(nanosec)\n')
		for t,c in zip(times,tdiff_ns):
			fdump.write('%d %d\n' % (t, c))
		fdump.close()

	# Filter outliers
	if True:
		std = np.std(tdiff_ns)
		med = np.median(tdiff_ns)
		mad = median_abs_deviation(tdiff_ns)
		flt_t, flt_c = [], []
		for t,c in zip(times,tdiff_ns):
			if (c >= (med - 10*mad)) and (c <= (med + 10*mad)):
				flt_t.append(t)
				flt_c.append(c)
		times = np.array(flt_t)
		tdiff_ns = np.array(flt_c)  

	# Linear fit
	# linfit = np.polyfit(times, tdiff_ns, 1)
	linfit = np.polyfit(flt_t, flt_c, 1)
	rate = linfit[0]*1e-9
	print('Linear-fit drift rate: %.6e s/s' % (rate))

	# Quadratic fit
	qfit = np.polyfit(flt_t, flt_c, 2)
	qrate, qaccel = qfit[0]*1e-9, qfit[1]*1e-18
	print('Quadratic-fit drift rate: %.6e s/s  acceleration: %.6e s/s^2' % (qrate,qaccel))

	# Plot data if possible
	try:
		import matplotlib
		matplotlib.use('TkAgg')
		import pylab
		import matplotlib.dates as md
		import datetime as dt
		havePlot = True
	except:
		havePlot = False

	if havePlot:
		med = np.median(tdiff_ns)
		mad = median_abs_deviation(tdiff_ns)
		linfit_tdiff_ns = np.polyval(linfit, times)
		qfit_tdiff_ns = np.polyval(qfit, times)

		unixdateconv = np.vectorize(dt.datetime.fromtimestamp)
		dttimes = unixdateconv(times)

		fig = pylab.figure()
		fig.patch.set_facecolor('white')

		fig.add_subplot(2,1,1)
		pylab.hold(True)
		pylab.plot(dttimes, tdiff_ns, 'x-', color='#666666')
		pylab.hold(True)
		pylab.plot(dttimes, linfit_tdiff_ns, '-', color='#660000')
		pylab.plot(dttimes, qfit_tdiff_ns, '-', color='#006600')
		pylab.ylim(med-3*mad, med+3*mad)
		pylab.text(dttimes[len(dttimes)//2], linfit_tdiff_ns[-1], 'rate %.4e s/s' % (rate), color='#660000')
		pylab.title(counter.key)
		pylab.ylabel('Time delta (nsec)')
		pylab.gca().get_xaxis().set_ticklabels([])

		fig.add_subplot(2,1,2)
		pylab.plot(dttimes, tdiff_ns - linfit_tdiff_ns, '+', color='#996666')
		pylab.plot(dttimes, tdiff_ns - qfit_tdiff_ns, 'x', color='#669966')
		pylab.plot([dttimes[0], dttimes[-1]], [0, 0], 'k-')
		ymax = np.max(np.abs(pylab.ylim()))
		pylab.ylim([-ymax, ymax])
		pylab.ylabel('Fit residual (nsec)')

		pylab.xticks(rotation=25, ha='right', fontsize=8)
		xfmt = md.DateFormatter('%Yy%jd%Hh%Mm')
		pylab.gca().xaxis.set_major_formatter(xfmt)


		pylab.tight_layout()
		pylab.draw()
		pylab.show()
