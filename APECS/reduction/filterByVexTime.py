#!/usr/bin/env python
'''
Filters the content of *.csv metadata files.

Returns lines that match a specific time range
and a part of a backend name like AP-N90. The
time range is read from a VEX file.
'''

from types import *
from support.timestamp import timestamp
from string import replace, join

import os, re, time, sys
import argparse
import datetime

__author__ = "Jan Wagner (MPIfR)"
__version__ = "1.0.0"

def parse_args(args):
	cmd = '%(prog)s <options>'
        parser = argparse.ArgumentParser(description=__doc__, add_help=True, formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument('--version', action='version', version='%(prog)s ' + __version__)
	parser.add_argument('-b', '--backend', dest='backend', default='AP-N90', help='substring of backend name to match (default: %(default)s)')
	parser.add_argument('-o', '--output', dest='output', default='filtered.csv', help='output file name (default: %(default)s)')
	parser.add_argument('vexfile')
	parser.add_argument('csvfiles', nargs='+')

	return parser.parse_args(args)


def gildas2datetime(tgildas):
	'''Convert an Gildas timestamp string (like 23-SEP-2026,14.195136791951) into Python datetime'''
	(normal_t, fractional_h) = tgildas.split(',')
	t = datetime.datetime.strptime(normal_t, '%d-%b-%Y') + datetime.timedelta(hours=float(fractional_h))
	return t


def vex2datetime(tvex):
	'''Convert a VEX timestamp string (like 2015y016d07h30m00s) into Python datetime'''
	return datetime.datetime.strptime(tvex, '%Yy%jd%Hh%Mm%Ss')


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

if __name__ == "__main__":

	opts = parse_args(sys.argv[1:])

	tstart, tstop = extractVexTimerange(opts.vexfile)
	if not (tstart and tstop):
		print("No time range found in VEX.")
		print("Currently got tstart of '%s', tstop of '%s'" % (str(tstart),str(tstop)))
		sys.exit(0)

	tstart_dt = vex2datetime(tstart)
	tstop_dt = vex2datetime(tstop)
	nfound = 0

	print("Looking up '%s'* data between '%s' and '%s'" % (opts.backend,str(tstart_dt),str(tstop_dt)))

	with open(opts.output, 'w') as outfile:

		for csvname in opts.csvfiles:
			with open(csvname, 'r') as csvf:
				print("filtering %s" % (csvname))
				for line in csvf:
					if len(line) < 30:
						continue
					if opts.backend not in line:
						continue
					t = gildas2datetime(line[0:24])
					if t < tstart_dt or t > tstop_dt:
						continue
					outfile.write(line)
					nfound += 1

		print("Wrote %d matching records into '%s'" % (nfound, opts.output))

