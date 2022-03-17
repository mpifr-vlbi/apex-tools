#!/usr/bin/env python
'''
Usage: vex2apecs.py <vexfile> <siteID>

Produces an .obs file that contains timed APECS commands for the VLBI observation.
The .obs file can later be run with apecsVLBI.py. Does not yet support frequency
changes during an observation (different MODEs in VEX).
'''

import sys
import datetime
import ntpath
## from VexLib import vex # had to give up on this, uses ply.lex, outdated...  now ugly parsing below...

src_file = 'vlbi-sources.cat'
lin_file = 'vlbi-freqs.lin'
rx_setup = 'NFLASH230_setup.apecs'
rx_commands = 'vlbi-NFLASH230_commands.py'

always_send_source = True  # set to True to always send source() go() track() for every scan, rather than only at each source change

def coordReformat(s):
	s = s.strip()
	repl = list('hmd\'')
	for c in repl:
		s = s.replace(c,':')
	s = s.replace('s','')
	s = s.replace('"','')
	return s
	# Note APECS wants reformatting
	# RA:  VEX 13h25m27.6152000s  --> APECS 13:25:27.6152000
	# DEC: VEX -43d01\'08.805000" --> APECS -43:01:08.80500

def getAllSources(fn):
	currSrc = ''
	sources = {}
	f = open(fn, 'r')
	content = f.readlines()
	f.close()
	for line in content:
		line = line.strip()
		if (len(line) < 1) or (line[0] == '*'):
			continue
		if 'source_name' in line:
			v = line.split('=')[1]
			currSrc = v.strip()[:-1]
		elif 'ref_coord_frame = J2000' in line:
			v = line.split(';')
			# v = ['ra = 17h33m02.7057870s', ' dec = -13d04\'49.548400"', ' ref_coord_frame = J2000', '']
			ra = coordReformat(v[0].split('=')[1])
			dec = coordReformat(v[1].split('=')[1])
			print ('%-20s  EQ  2000  %17s %17s   LSR 0.0' % (currSrc,ra,dec))
			entry = {}
			entry['ra'] = ra
			entry['dec'] = dec
			sources[currSrc] = entry
	return sources
		

def datetime2SNP(t):
	# Example SNP timestamp: 2015.016.07:30:00
	return t.strftime('%Y.%j.%H:%M:%S')

def timedelta2MinsSecs(dt):
	mins, secs = divmod(dt.total_seconds(), 60)
	return "%d min %02d sec" % (mins,secs)

def VEX2datetime(tvex):
	# Example VEX timestamp: 2015y016d07h30m00s
	return datetime.datetime.strptime(tvex, '%Yy%jd%Hh%Mm%Ss')

def getAllScans(site_ID, fn):
	key = 'station=%s' % (site_ID)
	scans = {}
	currScan = ''; currSource = ''; currStart = ''
	f = open(fn, 'r')
	content = f.readlines()
	f.close()
	for line in content:
		# scan No0081;
		#     start=2018y112d14h05m00s; mode=1mmlcp; source=J1924-2914;
		#     station=Ax:    0 sec:  180 sec:34153.754 GB:   :       : 1;
		#     station=Sz:    0 sec:  180 sec:25723.008 GB:   :       : 1;
		#     station=Lm:    0 sec:  180 sec:27446.082 GB:   :       : 1;
		# endscan;
		line = line.strip()
		if (len(line) < 1) or (line[0] == '*'):
			continue
		elif 'endscan' in line:
			currScan = ''
			currStart = ''
			currSource = ''
			continue
		elif 'scan ' in line:
			currScan = line[5:-1]
			continue
		elif len(currScan)>0:
			if 'start=' in line:
				tmp = line.replace(';','=')
				items = tmp.split('=')  # ['start', '2018y112d14h05m00s', ' mode', '1mmlcp', ' source', 'J1924-2914', '']
				currStart = items[1]
				currSource = items[5]
			elif key in line:
				if line[8:10] == site_ID:
					items = line.split(':')
					dur = int(items[2].replace('sec',''))
					entry = {}
					entry['start'] = VEX2datetime(currStart)
					entry['source'] = currSource
					entry['dur'] = dur
					scans[currScan] = entry
	return scans

def obs_writeLine(fd, time,dur,cmd):
	fd.write('%-22s %-10s %s\n' % (time,str(dur),cmd))

def obs_writeHeader(fd,site_ID,vexfn):
	T = datetime.datetime.utcnow()
	T = datetime2SNP(T)
	fd.write('#\n')
	fd.write('# APEX observing script for station %s from file %s\n' % (site_ID,vexfn))
	fd.write('# File created on %s UT\n' % (T))
	fd.write('#\n')
	fd.write('# Columns: 1) Start time 2) Duration in seconds 3) APECS command \n')
	fd.write('# Details on the columns:\n')
	fd.write('#    1) @always, or UT date-time in a 2015.016.06:42:40 format (yyyy.doy.hh:mm:ss)\n')
	fd.write('#         modifiers: !2015.016.06:42:40 to not skip command even if start time already is past\n')
	fd.write('#    2) estimated duration of the command in seconds\n')
	fd.write('#    3) APECS command and parameters to execute. If the command includes whitespace.\n')
	fd.write('#       Commands include, e.g.: tsys(), interactive("message"), tracksource("sourcename"), ...\n')
	fd.write('#\n')
	fd.write('# %-20s %-10s %s\n' % ('At time', 'Duration', 'Command'))

def obs_writeFooter(fd):
	fd.write('%s\n' % (80*'#'))
	fd.write('### end of schedule\n')
	fd.write('%s\n' % (80*'#'))

def obs_writeStandardsetup(fd):
	obs_writeLine(fd, '@always',  10, 'execfile(\'%s\')' % (rx_setup))
	obs_writeLine(fd, '@always',  2, 'execfile(\'%s\')' % (rx_commands))
	fd.write('\n')

def obs_writeScans(fd,scans,sources):

	Lslew    = 25  # max slew time in seconds to a new source
	Ltsys    = 20  # max time for Tsys sky/hot/cold measurement in seconds
	# Lrefscan = 70  # max time for on() scan with Off-Source reference
	Lrefscan = 25  # 2x10s + margin 5 max time for on() scan with Off-Source reference
	Lcmdmargin = 5 # allow 5 seconds between issuing any new APECS command
	L_minimum_for_interactive = 100 # seconds required to allow interactive input from observer
	Lschedule_lead_time = 180 # seconds to start schedule with first calibrations before first VLBI scan

	Tstart_next = None  # keep track of starting time of scan after current one
	prev_source = None  # keep track of source changes
	Tprev_end = None

	# Get scan names, start times, and sort by start time
	scannames = [key for key in scans.keys()]
	starttimes = [scans[sn]['start'] for sn in scannames]
	isorted = [starttimes.index(st) for st in sorted(starttimes)]

	# Generate APECS command entries for all scans
	for ii in isorted:
		scanname = scannames[ii]
		scan = scans[scanname]

		T = scan['start']
		T_scan = scan['start']
		Ldur = scan['dur']
		T_scan_end = T_scan + datetime.timedelta(seconds=Ldur)
		if Tprev_end==None:
			# First scan; virtual end of "previous" scan
			Tprev_end = T_scan - datetime.timedelta(seconds=Lschedule_lead_time)

		# Determine scan after current one (time margin, source name)
		if isorted.index(ii) < (len(isorted)-1):
			iinext = isorted[isorted.index(ii)+1]
			nextscan = scans[scannames[iinext]]
			Tstart_next = nextscan['start']
			# Time delta from end of current scan to start of next scan
			dT_to_next = (Tstart_next - T).total_seconds()
			dT_to_next = dT_to_next - Ldur
		else:
			nextscan = None
			Tstart_next = None
			dT_to_next = 9999999

		# Determine what things to do during current scan
		do_sourcechange = False
		do_vlbi_reference_scan = False
		do_vlbi_tsys = False
		L_start_early = 0

		if (isorted.index(ii) == 0) or (scan['source'] != prev_source):
			do_sourcechange = True
			L_start_early = L_start_early + Lslew + Lcmdmargin

		if (dT_to_next - L_start_early) > Ltsys:
			do_vlbi_tsys = True
			L_start_early = L_start_early + Ltsys + Lcmdmargin

		if (dT_to_next - L_start_early) > Lrefscan:
			do_vlbi_reference_scan = True
			L_start_early = L_start_early + Lrefscan + Lcmdmargin

		# Queue up the APECS commands to do for this scan
		# Structure: [prev.VLBIScan] -> change source -> vlbi_tsys() -> vlbi_reference_scan() -> [gap] -> VLBIScan
		# Old structure of EHT2018: [prev.VLBIScan] -> [gap] -> change source -> vlbi_tsys() -> vlbi_reference_scan() -> VLBIScan
		sheading = 'Target %s scanname %s to start at %s' % (scan['source'],scanname,datetime2SNP(scan['start']))
		fd.write('#### %s %s\n' % (sheading, '#'*(80-6-len(sheading))))
		T = Tprev_end + datetime.timedelta(seconds=10) # move to source right after previous VLBI scan
		if do_sourcechange or always_send_source:
			obs_writeLine(fd, datetime2SNP(T), Lslew, 'source(\'%s\'); go(); track()' % (scan['source']))
			T = T + datetime.timedelta(seconds=(Lslew+Lcmdmargin))
		if do_vlbi_tsys:
			obs_writeLine(fd, datetime2SNP(T), Ltsys, 'vlbi_tsys()')
			T = T + datetime.timedelta(seconds=(Ltsys+Lcmdmargin))
		if do_vlbi_reference_scan:
			obs_writeLine(fd, datetime2SNP(T), Lrefscan, 'vlbi_reference_scan()')
			T = T + datetime.timedelta(seconds=(Lrefscan+Lcmdmargin))

		Loperatorgap = T_scan - T
		if Loperatorgap.total_seconds() >= L_minimum_for_interactive:
			fd.write('#                      OPERATOR   vlbi calibrations likely end at %s\n' % datetime2SNP(T))
			fd.write('#                                 free %s until VLBI scan\n' % (timedelta2MinsSecs(Loperatorgap)))
		else:
			fd.write('#                      gap        max ~%s\n' % (timedelta2MinsSecs(Loperatorgap)))

		obs_writeLine(fd, datetime2SNP(T_scan), scan['dur'], 'vlbi_scan(t_mins=%d,targetSource=\'%s\')' % (scan['dur']/60,scan['source']))  # track on source
		T = T + datetime.timedelta(seconds=Ldur)
		fd.write('#     scan ends at %s\n\n' % datetime2SNP(T_scan_end))

		# Keep history
		prev_source = scan['source']
		Tprev_end = T_scan_end  # nominal end of VLBI scan, sans calibration

	fd.write('\n# Finished VLBI schedule\n')
	obs_writeLine(fd, datetime2SNP(Tprev_end + datetime.timedelta(seconds=30)), 1, 'remote_control(\'off\')')


def run(args):
	global src_file, lin_file

	if (len(args) != 3):
		print(__doc__)
		sys.exit(-1)

	vexfile = args[1]
	site = args[2]

	sources = getAllSources(vexfile)
	scans = getAllScans(site, vexfile)
	if len(scans) < 1:
		print ('\nError: no scans found for %s in %s!\n' % (site,vexfile))
		return

	obsfile = ntpath.basename(vexfile)
	obsfile = obsfile.replace('.vex', '.apecs.obs')

	fd = open(obsfile, 'w')
	obs_writeHeader(fd,site,vexfile)
	obs_writeStandardsetup(fd)
	obs_writeScans(fd,scans,sources)
	obs_writeFooter(fd)
	fd.close()

	print ('\nDone. Created APECS observing file %s with %d scans.\n' % (obsfile,len(scans)))

run(sys.argv)
