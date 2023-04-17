#!/usr/bin/env python
'''
Usage: vex2apecs.py <vexfile>

Produces an .obs file that contains timed APECS commands for the VLBI observation.
The .obs file can later be run with apecsVLBI.py. Does not yet support frequency
changes during an observation (different MODEs in VEX).
'''

import sys
import datetime
import ntpath
## from VexLib import vex # had to give up on this, uses ply.lex, outdated, now ugly manual parsing

rx_name = 'NFLASH230'

always_send_source = False  # set to True to always send source() go() track() for every scan, rather than only at each source change


def coordReformatVex2Apecs(s):
	'''
	Reformat VEX-style coordinates into APECS format, i.e.
	VEX RA   13h25m27.6152000s  --> APECS   13:25:27.6152000
	VEX DEC -43d01'08.805000"   --> APECS  -43:01:08.80500
	'''
	s = s.strip()
	repl = list('hmd\'')
	for c in repl:
		s = s.replace(c,':')
	s = s.replace('s','')
	s = s.replace('"','')
	return s


def datetimeFromVEX(tvex):
	'''Parse a VEX timestamp such as 2015y016d07h30m00s'''
	return datetime.datetime.strptime(tvex, '%Yy%jd%Hh%Mm%Ss')


def datetimeToSNP_str(t):
	'''Convert datetime into a SNP timestamp like 2015.016.07:30:00'''
	return t.strftime('%Y.%j.%H:%M:%S')


def timedelta2MinsSecs_str(dt):
	'''Convert timedelta into a string'''
	mins, secs = divmod(dt.total_seconds(), 60)
	return "%d min %02d sec" % (mins,secs)


# ===========================================================================================
# VEX Parsing
# ===========================================================================================

def getAllSources(vexfilename):
	'''
	Read VEX file and return a dict of all sources and their Ra Dec coordinates
	dict[source name] = {'ra':RA, 'dec':Dec}
	'''
	srcName = ''
	sources = {}
	f = open(vexfilename, 'r')
	content = f.readlines()
	f.close()
	for line in content:
		line = line.strip()
		if (len(line) < 1) or (line[0] == '*'):
			continue
		if 'source_name' in line:
			v = line.split('=')[1]
			srcName = v.strip()[:-1]
		elif 'ref_coord_frame = J2000' in line:
			v = line.split(';')
			# v = ['ra = 17h33m02.7057870s', ' dec = -13d04\'49.548400"', ' ref_coord_frame = J2000', '']
			ra = coordReformatVex2Apecs(v[0].split('=')[1])
			dec = coordReformatVex2Apecs(v[1].split('=')[1])
			print ('%-20s  EQ  2000  %17s %17s   LSR 0.0' % (srcName,ra,dec))
			entry = {}
			entry['ra'] = ra
			entry['dec'] = dec
			sources[srcName] = entry
	return sources


def getAllScans(vexfilename, site_ID):
	'''
	Read VEX file and return a dict of all scans with a specific station
	dict[scan name] = {'start':datetime, 'dur':duration_secs, 'source':name of source}
	'''
	f = open(vexfilename, 'r')
	content = f.readlines()
	f.close()

	currScan, currSource, currStart = '', '', ''
	scans = {}

	key = 'station=%s' % (site_ID)

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
					entry['start'] = datetimeFromVEX(currStart)
					entry['source'] = currSource
					entry['dur'] = dur
					scans[currScan] = entry
	return scans



# ===========================================================================================
# Timed command file (.obs) writing
# ===========================================================================================


def obs_writeHeader(fd,site_ID,vexfilename):
	T = datetime.datetime.utcnow()
	T = datetimeToSNP_str(T)
	fd.write('#\n')
	fd.write('# APEX observing script for station %s from file %s\n' % (site_ID,vexfilename))
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


def obs_writeComment(fd, comment):
	fd.write('# %s\n' % (comment))


def obs_writeLine(fd, time, dur,cmd):
	fd.write('%-22s %-10s %s\n' % (time,str(dur),cmd))


def obs_writeStandardsetup(fd, rxName='NFLASH230'):
	rx_setup = '%s_setup.apecs' % (rxName)
	rx_commands = 'vlbi-%s_commands.py' % (rxName)
	obs_writeLine(fd, '@always', 10, 'execfile(\'%s\')' % (rx_setup))
	obs_writeLine(fd, '@always', 2, 'execfile(\'%s\')' % (rx_commands))
	fd.write('\n')


def obs_writeScans(fd,scans,sources):

	Lslew    = 20		# max slew time in seconds to a new source (TODO: take into account slew rates and angular separation!)
	Ltsys    = 80		# max time for Tsys sky/hot/cold measurement in seconds - EHT2022: calibrate(mode='HOT',time=6)  takes 65 sec
	#Ltsys_no_cold = 40	# max time for Tsys sky/hot measurement in seconds - EHT2022: calibrate(mode='COLD',time=6) takes 40 sec, calibrate(..,time=10) takes ~80 sec
	Ltsys_no_cold = 22	# -"- but lie about duration, allowing short Tsys in e22b19 50sec gaps (30sec after slew!) and accept first 10 sec of vlbi data are likely on Hot load
	Lrefscan = 30  		# EHT2022: 2x10s + margin 10 max time for on() scan with Off-Source reference
	Lcmdmargin = 5		# allow n seconds between issuing any new APECS command
	L_minimum_for_interactive = 6*60 # seconds required to allow interactive input from observer - EHT2023: 6 minutes
	Lschedule_lead_time = 200 # seconds to start schedule with first calibrations before first VLBI scan

	Tstart_next = None  # keep track of starting time of scan after current one
	prev_source = None  # keep track of source changes
	Tprev_end = None
	scanNr = 1

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
			gap_to_next = (Tstart_next - T_scan_end).total_seconds()
			gap_to_curr = (T_scan - Tprev_end).total_seconds()
		else:
			nextscan = None
			Tstart_next = None
			gap_to_next = 9999999
			gap_to_curr = Lschedule_lead_time

		# Label for scan
		sheading = "Block for %s scan %d '%s' at %s in %ds" % (scan['source'],scanNr,scanname,datetimeToSNP_str(scan['start']),gap_to_curr)

		# Determine what things to do for the current scan,
 		# i.e., make a list of pre-obs and on-scan functions to call.

		do_sourcechange = False
		do_vlbi_reference_scan = False
		do_vlbi_tsys = False
		do_vlbi_tsys_shorter = False
		L_prescan_tasks = 0

		# Plan what to do, time permitting

		if (isorted.index(ii) == 0) or (scan['source'] != prev_source) or always_send_source:
			do_sourcechange = True
			L_prescan_tasks += Lslew + Lcmdmargin

		if (gap_to_curr - L_prescan_tasks - Lcmdmargin) > Ltsys:
			do_vlbi_tsys = True
			L_prescan_tasks += Ltsys + Lcmdmargin
		elif (gap_to_curr - L_prescan_tasks - Lcmdmargin) > Ltsys_no_cold:
			do_vlbi_tsys_shorter = True
			L_prescan_tasks += Ltsys_no_cold + Lcmdmargin
		else:
			print('Warning: pre-scan cals for %s (%s) must omit Tsys, time margin is only %d sec, need >%d sec' % (scanname, scan['source'], gap_to_curr - L_prescan_tasks - Lcmdmargin, Ltsys_no_cold))

		if (gap_to_curr - L_prescan_tasks - Lcmdmargin) > Lrefscan:
			do_vlbi_reference_scan = True
			L_prescan_tasks += Lrefscan + Lcmdmargin
		else:
			print('Warning: pre-scan cals for %s (%s) must omit reference scan, time margin is only %d sec, need >%d sec' % (scanname, scan['source'], gap_to_curr - L_prescan_tasks - Lcmdmargin, Lrefscan))

		# Assemble the calibration commands determined above

		obs_writeComment(fd, '### %s %s' % (sheading, '#'*(80-6-len(sheading))))

		calBlock = []
		calTotalTime = 0

		if do_sourcechange or always_send_source:
			calstep = {'offset':calTotalTime, 'dur':Lslew, 'cmd':'source(\'%s\',cats=\'user\'); go(); track()' % (scan['source'])}
			calTotalTime += Lslew + Lcmdmargin
			calBlock.append(calstep)

		if do_vlbi_tsys_shorter:
			calstep = {'offset':calTotalTime, 'dur':Ltsys_no_cold, 'cmd':"vlbi_tsys(mode_='HOT',time_s=5)"}
			calTotalTime += Ltsys_no_cold + Lcmdmargin
			calBlock.append(calstep)
		elif do_vlbi_tsys:
			calstep = {'offset':calTotalTime, 'dur':Ltsys, 'cmd':'vlbi_tsys()'}
			calTotalTime += Ltsys + Lcmdmargin
			calBlock.append(calstep)

		if do_vlbi_reference_scan:
			calstep = {'offset':calTotalTime, 'dur':Lrefscan, 'cmd':'vlbi_reference_scan()'}
			calTotalTime += Lrefscan + Lcmdmargin
			calBlock.append(calstep)

		# Place the calibration commands
		#
		# A) [prev VLBI Scan] -> change source -> vlbi_tsys() -> vlbi_reference_scan() -> [short gap] -> [VLBI Scan]
		# B) [prev VLBI Scan] -> [long gap] -> change source -> vlbi_tsys() -> vlbi_reference_scan() -> [VLBI Scan]

		Loperatorgap = datetime.timedelta(seconds=gap_to_curr - calTotalTime)
		immediateCal = Loperatorgap.total_seconds() < (60 + 6*Lcmdmargin)
		allowInteractive = Loperatorgap.total_seconds() >= L_minimum_for_interactive

		if immediateCal:
			T_cal_start = Tprev_end
		else:
			T_cal_start = T_scan - datetime.timedelta(seconds=calTotalTime+2*Lcmdmargin)
		T_cal_end = T_cal_start + datetime.timedelta(seconds=calTotalTime)

		if (not immediateCal) and allowInteractive:
			obs_writeComment(fd, '')
			obs_writeComment(fd, 'OPERATOR free %s until VLBI calibrations to start at %s' % (timedelta2MinsSecs_str(Loperatorgap),datetimeToSNP_str(T_cal_start)))
			obs_writeComment(fd, '')
			note = 'You have %s until VLBI pre-scan calibrations at %s. Remember to do remote_control ON.' % (timedelta2MinsSecs_str(Loperatorgap),datetimeToSNP_str(T_cal_start))
			obs_writeLine(fd, datetimeToSNP_str(Tprev_end), 1, 'remote_control(\'off\'); print(\'%s\')' % (note))
	
		for calEntry in calBlock:
			calT = T_cal_start + datetime.timedelta(seconds=calEntry['offset'])
			obs_writeLine(fd, datetimeToSNP_str(calT), calEntry['dur'], calEntry['cmd'])

		if immediateCal or True:
			LpostCalGap = T_scan - T_cal_end
			if LpostCalGap.total_seconds() > 0:
				obs_writeComment(fd, '                     gap        max ~%s' % (timedelta2MinsSecs_str(LpostCalGap)))
			else:
				obs_writeComment(fd, '                     TIME       ran short, ~%s "gap"' % (timedelta2MinsSecs_str(LpostCalGap)))

		# Place VLBI scan command

		obs_writeLine(fd, datetimeToSNP_str(T_scan), scan['dur'], 'vlbi_scan(t_mins=%d,targetSource=\'%s\')' % (scan['dur']/60,scan['source']))
		obs_writeComment(fd, '    scan ends at %s\n' % (datetimeToSNP_str(T_scan_end)))

		T = T + datetime.timedelta(seconds=Ldur)

		# Keep history

		prev_source = scan['source']
		Tprev_end = T_scan_end  # nominal end of VLBI scan, sans calibration
		scanNr += 1

	fd.write('\n# Finished VLBI schedule\n')
	obs_writeLine(fd, datetimeToSNP_str(Tprev_end + datetime.timedelta(seconds=30)), 1, 'remote_control(\'off\')')


def run(args):

	if (len(args) != 2):
		print(__doc__)
		sys.exit(-1)

	site = 'Ax'
	rxName = 'NFLASH230'

	vexfile = args[1]

	sources = getAllSources(vexfile)
	scans = getAllScans(vexfile, site)
	if len(scans) < 1:
		print ('\nError: no scans found for %s in %s!\n' % (site,vexfile))
		return

	obsfile = ntpath.basename(vexfile)
	obsfile = obsfile.replace('.vex', '.apecs.obs')

	fd = open(obsfile, 'w')
	obs_writeHeader(fd,site,vexfile)
	obs_writeStandardsetup(fd,rxName)
	obs_writeScans(fd,scans,sources)
	obs_writeFooter(fd)
	fd.close()

	print ('\nDone. Created APECS observing file %s with %d scans.\n' % (obsfile,len(scans)))


run(sys.argv)
