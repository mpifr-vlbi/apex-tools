#!/usr/bin/env python

from VexLib import vex
import sys
import datetime

def usage():
	print ''
	print 'Usage: vex2apecs.py <vexfile> <siteID>'
	print ''
	print 'Produces an .obs file that contains timed APECS commands for the VLBI observation.'
	print 'The .obs file can later be run with apecsVLBI.py'
	print ''

def datetime2SNP(t):
	# Example SNP timestamp: 2015.016.07:30:00
	return t.strftime('%Y.%j.%H:%M:%S')

def VEX2datetime(snp):
	# Example VEX timestamp: 2015y016d07h30m00s
	return datetime.datetime.strptime(snp, '%Yy%jd%Hh%Mm%Ss')

def getSite(site, v):
	site_name = None
	site_ID = None
	for s in v['SITE']:
		if (site == s):
			site_name = s
			site_ID = v['SITE'][s]['site_ID']
			continue
		if (site == v['SITE'][s]['site_ID']):
			site_name = s
			site_ID = site
	return (site_name,site_ID)

def getSource(source_name, v):
	sourceinfo = None
	for src in v['SOURCE']:
		if not((src == source_name) or (v['SOURCE'][src]['source_name'] == source_name)):
			continue

		s = v['SOURCE'][src]

		# Convert VEX coordinate strings into APECS coordinate strings
		# RA: VEX 13h25m27.6152000s --> APECS 13:25:27.6152000
		ra = v['SOURCE'][src]['ra']  
		ra = ra.replace('h', ':')
		ra = ra.replace('m', ':')
		ra = ra.replace('s', '')
		# DEC: VEX -43d01\'08.805000" --> APECS -43:01:08.80500
		dec = v['SOURCE'][src]['dec'] 
		dec = dec.replace('d', ':')
		dec = dec.replace('\'', ':')
		dec = dec.replace('"', '')

		si = {}
		si['source'] = v['SOURCE'][src]['source_name']
		si['eq']  = v['SOURCE'][src]['ref_coord_frame'][1:]
		si['ra']  = ra
		si['dec'] = dec

		sourceinfo = si
		break

	return sourceinfo

def getScans(site_ID, v):
	scans = []
	for s in v['SCHED']:
		scaninfo = None
		stations = v['SCHED'][s].getall('station')
		found_match = False
		for st in stations:
			if (st[0] == site_ID):
				scaninfo = {}
				scaninfo['name']    = s
				scaninfo['start']   = VEX2datetime(v['SCHED'][s]['start'])
				scaninfo['source']  = getSource(v['SCHED'][s]['source'], v)
				scaninfo['mode']  = v['SCHED'][s]['mode']
				scaninfo['pre_sec'] = st[1].replace(' sec','')  # the VEX 'data good after' field
				scaninfo['dur_sec'] = st[2].replace(' sec','')  # scan length in seconds
				scaninfo['rec_len'] = st[3]  # typically GByte
				scans.append(scaninfo)
				found_match = True
				break
		if found_match:
			print 'Scan %s : added' % (s)
		else:
			print 'Scan %s does not include station %s' % (s, site_ID)
	return scans

def obs_writeLine(fd, time,dur,cmd):
	fd.write('%-22s %-10s %s\n' % (time,str(dur),cmd))

def obs_writeHeader(fd,site_name,site_ID,v):
	T = datetime.datetime.utcnow()
	T = datetime2SNP(T)
	expname = v['GLOBAL']['EXPER']
	exp = v['EXPER'][expname]
	Tstart = 'n/a'; Tstop = 'n/a'; PI = 'n/a'
	try:
		Tstart = exp['exper_nominal_start']
		Tstop = exp['exper_nominal_stop']
		PI = exp['PI_email']
	except Error:
		pass
	fd.write('#\n')
	fd.write('# APEX observing script for station %s (%s), experiment %s\n' % (site_name,site_ID,expname))
	fd.write('# Experiment starts %s, ends %s. PI contact: %s\n' % (Tstart,Tstop,PI))
	fd.write('# File created on %s UT\n' % (T))
	fd.write('#\n')
	fd.write('# Columns: 1) Start time 2) Duration in seconds 3) APECS command \n')
	fd.write('# Details on the columns:\n')
	fd.write('#    1) @always, or UT date-time in a 2015.016.06:42:40 format (yyyy.doy.hh:mm:ss)\n')
	fd.write('#         modifiers: !2015.016.06:42:40 to not skip command even if start time already is past\n')
	fd.write('#    2) estimated duration of the command in seconds\n')
	fd.write('#    3) APECS command and parameters to execute. If the command includes whitespace, use\n')
	fd.write('#       quotes around the command ("cmd").\n')
	fd.write('#       Commands include, e.g.: tsys(), interactive("message"), tracksource("sourcename"), ...\n')
	fd.write('#\n')
	fd.write('# %-20s %-10s %s\n' % ('Time', 'Duration', 'Command'))

def obs_writeFooter(fd):
	fd.write('%s\n' % (80*'#'))
	fd.write('### end of schedule\n')
	fd.write('%s\n' % (80*'#'))

def obs_writeStandardsetup(fd):
	obs_writeLine(fd, '@always', 10, 'vlbi_clockoffsets()')
	obs_writeLine(fd, '@always', 10, 'vlbi_tuning()')

def obs_writeScans(fd,scans):
	Nscans = len(scans)

	Lpre    = 15  # preobs  x seconds before scan
	Lpost   =  5  # postobs x seconds after scan
	Ltsys   = 50  # seconds it takes for APECS to do a calibrate()
	Lmeters = 15  # seconds it takes to read clock offsets, PWV, WX data
	L_minimum_for_interactive = 600 # seconds required to allow interactive input from observer

	for ii in range(Nscans):
		si = scans[ii]
		if (ii>=1):
			Tstart_prev = scans[ii-1]['start']
		else:
			Tstart_prev = None
		if (ii<(Nscans-1)):
			Tstart_next = scans[ii+1]['start']
		else:
			Tstart_next = None

		sheading = '%s/%s/%s' % (si['name'],si['source']['source'],si['mode'])
		fd.write('#### %s %s\n' % (sheading, '#'*(80-6-len(sheading))))

		T = si['start']
		Ldur = int(si['dur_sec'])

		T = T + datetime.timedelta(seconds=-Lpre)
		obs_writeLine(fd, datetime2SNP(T), Lpre-5, 'source(..TODO..)')
		T = T + datetime.timedelta(seconds=Lpre-5)
		obs_writeLine(fd, datetime2SNP(T), 5, 'doppler(\'off\')')

		T = T + datetime.timedelta(seconds=5)
		obs_writeLine(fd, datetime2SNP(T), si['dur_sec'], 'track()')
		T = T + datetime.timedelta(seconds=Ldur)

		T = T + datetime.timedelta(seconds=Lpost)
		obs_writeLine(fd, datetime2SNP(T), Ltsys, 'calibrate()')

		T = T + datetime.timedelta(seconds=Ltsys+5)
		obs_writeLine(fd, datetime2SNP(T), Lmeters, 'readMeters()')

		if (Tstart_next != None):
			Lscangap = (Tstart_next - si['start'])
			Lscangap = Lscangap - datetime.timedelta(seconds=Ldur)
			Lscangap = Lscangap - datetime.timedelta(seconds=Lpost)
			Lscangap = Lscangap - datetime.timedelta(seconds=Ltsys+5)
			Lscangap = Lscangap - datetime.timedelta(seconds=Lmeters+5)
			Lscangap = Lscangap.total_seconds()
			fd.write('#     %s seconds until next scan\n' % (str(Lscangap)) )

def run(args):
	if (len(args) != 3):
		usage()
		sys.exit(-1)

	vexfile = args[1]
	site = args[2]

	fp = open(vexfile, 'r')
	v = vex.parse(fp.read())
	fp.close()

	obsfile = v['GLOBAL']['EXPER'] + site + '.obs'
	(site_name,site_ID) = getSite(site, v)

	if (site_name == None):
		print 'Could not find site %s in the VEX file.' % (site)
		sys.exit(-1)

	print 'Creating .obs file for site %s (full info: %s)' % (site_name,str(v['SITE'][site_name]))

	scans = getScans(site_ID, v)

	fd = open(obsfile, 'w')
	obs_writeHeader(fd,site_name,site_ID,v)
	obs_writeStandardsetup(fd)
	obs_writeScans(fd,scans)
	obs_writeFooter(fd)
	fd.close()

run(sys.argv)
