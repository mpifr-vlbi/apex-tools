#
# SNAP File Parser and APECS Observation Scheduler
#
# (C) 2013 Jan Wagner
#

import struct
import code
from time import sleep
from time import time
from socket import socket, AF_INET, SOCK_DGRAM
from datetime import datetime


# Use NTP server for current time because APECS systems run 
# in a quite strange time standard
m_ntpserver = 'nist1-lnk.binary.net'
m_PC_minus_NTP_offset = 0  # determined in function xxx(?) below

# Minimum time in seconds allowed before 'now' and next scan to consider for observing
m_min_time_to_next_scan = 30

# Minimum time gap between scans for user interaction
m_min_gap_for_userinteract = 3*60


""" Gets NTP time and returns a datetime object """
def send_ntp_request(ntpserver):
    try:
        sock = socket(AF_INET, SOCK_DGRAM)
        sock.settimeout(5)
        data = '\x1b' + 47 * '\0'
        start_time = time()
        sock.sendto(data, (ntpserver, 123))
        data, ip = sock.recvfrom(1024)
        req_time = time() - start_time
        if data:
            timestamp = struct.unpack('!12I', data)[10]
            timestamp -= 2208988800L # = date in sec since epoch
            timestamp += req_time / 2
            return datetime.utcfromtimestamp(timestamp)
    except:
        return 'ERROR: socket request failed'


""" Determine offset in seconds between local PC clock ntp and actual UTC NTP server 
    to a small degree of accuracy (e.g. plusminus a few seconds) """
def vlbi_observeSnap_getPCminusNTPoffset():

	global m_ntpserver

	print ''
	print 'PC UTC versus NTP UTC quick and dirty calibration...'

	while True:
		ntpUtcNow = send_ntp_request(m_ntpserver)
		if not(str(ntpUtcNow).startswith('ERROR')):
			break
		print 'NTP server did not reply, retrying in 5 seconds...'
		sleep(5.0)

	pcUtcNow = datetime.utcnow()

	ntpUtcNowTM = ntpUtcNow.timetuple()
	pcUtcNowTM = pcUtcNow.timetuple()

	pcUtcNowDoy = int(pcUtcNowTM.tm_yday)
	ntpUtcNowDoy = int(ntpUtcNowTM.tm_yday)

	pcUtcNowSecofyear = pcUtcNowDoy*24*60*60 + pcUtcNowTM.tm_hour*60*60 + pcUtcNowTM.tm_min*60 + int(pcUtcNowTM.tm_sec)
	ntpUtcNowSecofyear = ntpUtcNowDoy*24*60*60 + ntpUtcNowTM.tm_hour*60*60 + ntpUtcNowTM.tm_min*60 + int(ntpUtcNowTM.tm_sec)
	
	PC_minus_NTP_seconds = pcUtcNowSecofyear - ntpUtcNowSecofyear

	print 'NTP UTC: %s ' % (str(ntpUtcNow))
	print 'PC  UTC: %s ' % (str(pcUtcNow))
	print 'PC leads NTP by: %s seconds or %.2f minutes' % (PC_minus_NTP_seconds, int(PC_minus_NTP_seconds)/60.0)
	print ''

	m_PC_minus_NTP_offset = PC_minus_NTP_seconds

	return PC_minus_NTP_seconds


""" Wait until PC clock is equal to or greated than the specified UTC seconds-of-year,
    also taking into account an offset of PC UTC clock to actual UTC """
def vlbi_observeSnap_waitUntilPCatUTC(utc_secofyear, PCminusNTP_utc_sec):

	pcUtcNow = datetime.utcnow()
	pcUtcNowTM = pcUtcNow.timetuple()
	pcUtcNowDoy = int(pcUtcNowTM.tm_yday)
	pcUtcNowSecofyear = pcUtcNowDoy*24*60*60 + pcUtcNowTM.tm_hour*60*60 + pcUtcNowTM.tm_min*60 + int(pcUtcNowTM.tm_sec)
	pc_trueUTC = pcUtcNowSecofyear - PCminusNTP_utc_sec
	dT = utc_secofyear - pc_trueUTC

	print 'Waiting from current sec-of-year %s until %s (%s sec or %.1f min) minus PC UTC offset of %s seconds...' % \
		(pc_trueUTC, utc_secofyear, dT, dT/60.0, PCminusNTP_utc_sec)

	while True:
		pcUtcNow = datetime.utcnow()
		pcUtcNowTM = pcUtcNow.timetuple()
		pcUtcNowDoy = int(pcUtcNowTM.tm_yday)
		pcUtcNowSecofyear = pcUtcNowDoy*24*60*60 + pcUtcNowTM.tm_hour*60*60 + pcUtcNowTM.tm_min*60 + int(pcUtcNowTM.tm_sec)
	
		pc_trueUTC = pcUtcNowSecofyear - PCminusNTP_utc_sec
		if pc_trueUTC >= utc_secofyear:
			print '  -- time reached'
			break
		sleep(1.0)

	return

""" Loads info of all scans in SNP file """
def vlbi_loadSnap(snapfile):

	scans = []
	fid = open(snapfile, 'r')

	while True:

		scan = vlbi_observeSnap_getNextBlock(fid)
		if scan == {}:
			break
		scans = scans + [scan]

	for ss in scans:
		print 'start %s (%s) : %s : %s : dur %s sec' % (ss['starttime']['str'], \
			ss['starttime']['secofyear'], ss['source'], ss['filename'], ss['dur_sec'])
	print 'Found %d scans in total' % (len(scans))

	return scans


""" Converts "!2013.080.05:00:00" type of SNP file time into struct """
def vlbi_observeSnap_snapTime2struct(textline):
	stime = {}
	textline = textline.strip()
	if not(textline[0] == '!'):
		print 'Not a valid SNP time string!! (%s)' % (textline)
		return stime
	T = textline[1:]
	stime['str']  = T
	stime['year'] = int(T[0:4])
	stime['doy']  = int(T[5:8])
	stime['hour'] = int(T[9:11])
	stime['min']  = int(T[12:14])
	stime['sec']  = int(T[15:17])
	stime['secofyear'] = int( stime['sec'] + 60*stime['min'] + \
			60*60*stime['hour'] + 24*60*60*stime['doy'] )
	return stime

# Find next block in input file that starts with 'scan_name' and 
# that ends with 'postob'. Returns scan name, scan duration,
# start time and the SNP impression of the stop time (should
# be equal to start time plus scan duration, usually!)
def vlbi_observeSnap_getNextBlock(fid):

	block_started = False
	prev_time = {}
	scan = {}

	# <<Example>>
	# scan_name=No0001,d21us,Ap,240,240
	# source=0854+201,085448.87,200630.6,2000.0,
	# setup01
	# !2013.080.04:59:50
	# preob
	# !2013.080.05:00:00
	# mk5=record=on:d21us_ap_no0001;
	# data_valid=on
	# midob
	# !2013.080.05:04:00
	# data_valid=off
	# mk5=record=off;
	# postob

	while True:

		l = fid.readline()
		ll = l.strip()
		if not l:
			break

		# some time entry like '!1998.001.12:00:00'?
		if ll[0] == '!':
			prev_time = vlbi_observeSnap_snapTime2struct(ll)
			continue

		# end of block?
		if ll.startswith('postob'):
			scan['stoptime'] = prev_time
			scan['completed'] = False
			break

		# start of block?
		if not(block_started):

			if ll.startswith('scan_name'):
				sinfo = ll[10:]
				sinfo = sinfo.split(',')
				scan['name'] = sinfo[0]
				scan['expt'] = sinfo[1]
				scan['stn']  = sinfo[2]
				scan['dur_sec'] = sinfo[3]
				block_started = True

			continue

		# source?
		if ll.startswith('source='): 
			sr = ll[7:].split(',')
			scan['source'] = sr[0]

		# start of recording?
		if ll.startswith('mk5=record=on'):

			scan['starttime'] = prev_time
			scan['filename'] = ll[14:]
			continue
		
	return scan



""" Given a scan list, uses current NTP time to decide which scan is upcoming next """
def vlbi_observeSnap_selectUpcomingScan(scanlist):

	global m_ntpserver
	global m_min_time_to_next_scan

	scan = {}
	scanindex = -1

	while True:
		utcNow = send_ntp_request(m_ntpserver)
		if not(str(utcNow).startswith('ERROR')):
			break
		print 'NTP server did not reply, retrying in 5 seconds...'
		sleep(5.0)

	utcNowTM = utcNow.timetuple()
	utcNowDoy = int(utcNowTM.tm_yday)
	utcNowSecofyear = utcNowDoy*24*60*60 + utcNowTM.tm_hour*60*60 + utcNowTM.tm_min*60 + int(utcNowTM.tm_sec)

	print 'Looking for next scan observable from now with >=%s seconds margin until start:' % (m_min_time_to_next_scan)
	print '  Current UTC (NTP)   : %s' % (utcNow)
	print '  Current sec of year : %s' % (utcNowSecofyear)
	print '  Current doy (NTP)   : %s' % (utcNowDoy)

	# note: when read from a SNAP file, the start times are already sorted in ascending order
	for ss in scanlist:

		scanindex = scanindex + 1
		if ss['starttime']['secofyear'] < (utcNowSecofyear + m_min_time_to_next_scan):
			continue

		if ss['completed']:
			print '  -- skipping scan already marked as completed: %s at %s for %s sec' % \
				(ss['starttime']['str'], ss['source'], ss['dur_sec'])
			continue

		scan = ss
		break

	if (len(scan)==0):
		print '  -- no upcoming scans found!'
	else:
		print '  -- found: %s at %s for %s sec' % (scan['starttime']['str'], scan['source'], scan['dur_sec'])

	return (scan,scanindex)


""" Full fledged APECS auto-observing of scans in SNP file """
def vlbi_observeSnap(snapfile):

	# Determine our NTP-locked clock offset versus actual UTC NTP (APEX uses no leap seconds)
	PC_NTP_offset = vlbi_observeSnap_getPCminusNTPoffset()    # updates m_PC_minus_NTP_offset

	# Load scans
	scans = vlbi_loadSnap(snapfile)

	# Process all unobserved upcoming scans
	while True:

		# Get next unobserved scan that follows after the current UTC time

		(upcomingscan,scanindex) = vlbi_observeSnap_selectUpcomingScan(scans)
		if len(upcomingscan)==0:
			print 'No More Scans'
			break

		# Get time delta between end of current and start of next scan
		try:
			followingscan = scans[scanindex+1]
			last_scan = False
		except:
			followingscan = upcomingscan
			last_scan = True

		if last_scan:
			T_gap = 900
			print 'Last scan!'

		else:
			T_gap = followingscan['starttime']['secofyear'] - upcomingscan['stoptime']['secofyear']

			print 'Deciding scan gap interactivity:'
			print '   selected: %10s at %s for %s sec (%.1f min)' % \
				(upcomingscan['source'],upcomingscan['starttime']['str'], upcomingscan['dur_sec'], \
				int(upcomingscan['dur_sec'])/60.0)
			print '        end: %10s at %s' % (' ',upcomingscan['stoptime']['str'])
			print '   2nd next: %10s at %s' % (followingscan['source'],followingscan['starttime']['str'])
			print '             %10s gap = %s sec (%.1f min)' % (' ', T_gap, int(T_gap)/60.0)

		if (T_gap > m_min_gap_for_userinteract):
			print 'Time gap %s sec longer than minimum gap %s' % (T_gap,m_min_gap_for_userinteract)
			print 'Allowing user APECS interaction in gap'
			allow_interact = True
		else:
			print 'Time gap %s sec shorter than minimum gap %s' % (T_gap,m_min_gap_for_userinteract)
			print 'Observing this and next scan without user interaction'
			allow_interact = False


		# APECS go to source
		scan = upcomingscan
		print "APECS: source('%s')" % (scan['source'])
		source(scan['source'])
		print "APECS: track()"
		track()

		# Wait until scan start
		secstart = scan['starttime']['secofyear'] - 5
		vlbi_observeSnap_waitUntilPCatUTC(secstart, PC_NTP_offset)

		# APECS record total power on the source for length of VLBI scan
		print "APECS: on(drift='no',time=%s)" % (scan['dur_sec'])
		on(drift='no',time=scan['dur_sec'])
	
		# Wait a bit past scan duration
		secstop = scan['stoptime']['secofyear'] + 5
		vlbi_observeSnap_waitUntilPCatUTC(secstop, PC_NTP_offset)

		# Do a Sky-Hot-Cold
		print 'APECS: calibrate()'
		calibrate()

		if allow_interact:
			print '>>> User Interaction (skydip() etc) without time-out'
			print '>>> Be careful not to exceed scan gap time!'
			print '>>> When done type "cont" to continue schedule'
			#code.interact(local=locals())
			while True:
				l = raw_input('apecs_snp> ')
				if (l.lower().strip() == 'cont'):
					break
				eval(l, globals(), locals())
				try:
					eval(l, globals(), locals())
				except:
					pass

		# Mark as completed
		scans[scanindex]['completed'] = True

	# All scans done
	print 'SNAP file has been processed'


