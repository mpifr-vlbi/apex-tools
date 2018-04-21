#!/usr/bin/env python

import sys
import datetime
## from VexLib import vex # had to give up on this, uses ply.lex, outdated...  now ugly parsing below...

src_file = 'vlbi-sources.cat'
lin_file = 'vlbi-freqs.lin'

def usage():
	print ('')
	print ('Usage: vex2apecs.py <vexfile> <siteID>')
	print ('')
	print ('Produces an .obs file that contains timed APECS commands for the VLBI observation.')
	print ('The .obs file can later be run with apecsVLBI.py. Does not yet support frequency')
	print ('changes during an observation (different MODEs in VEX).')
	print ('')

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
	obs_writeLine(fd, '@always',  10, 'execfile(\'pi230_setup.apecs\')')
	obs_writeLine(fd, '@always',  2, 'execfile(\'vlbi-pi230_commands.py\')')

def obs_writeScans(fd,scans,sources):

	Lpre    = 10  # preobs  x seconds before scan
	Lpost   =  5  # postobs x seconds after scan
	Ltsys   = 50  # seconds it takes for APECS to do a calibrate()
	Lmeters = 15  # seconds it takes to read clock offsets, PWV, WX data
	L_minimum_for_interactive = 480 # seconds required to allow interactive input from observer
	Tstart_next = None

	scannames = scans.keys()
	scannames = sorted(scannames) # assumes that the alphabetic order is also the time order...

	for scanname in scannames:
		ii = scannames.index(scanname)
		scan = scans[scanname]

		sheading = 'Target %s scanname %s to start at %s' % (scan['source'],scanname,datetime2SNP(scan['start']))
		fd.write('#### %s %s\n' % (sheading, '#'*(80-6-len(sheading))))

		T = scan['start']
		Ldur = scan['dur']
		
		if ii < (len(scannames)-1):
			nextscan = scans[scannames[ii+1]]
			Tstart_next = nextscan['start']
		else:
			Tstart_next = None

		T = T + datetime.timedelta(seconds=-Lpre)

		obs_writeLine(fd, datetime2SNP(T), 5, 'doppler(\'off\')')
		T = T + datetime.timedelta(seconds=5)

		obs_writeLine(fd, datetime2SNP(T), Lpre-5, 'source(\'%s\')' % (scan['source']))
		T = T + datetime.timedelta(seconds=Lpre-5)

		obs_writeLine(fd, datetime2SNP(T), scan['dur'], 'vlbi_tp_onsource(src=\'%s\',t=%d)'  % (scan['source'],scan['dur']/60))
		T = T + datetime.timedelta(seconds=Ldur)

		T = T + datetime.timedelta(seconds=Lpost)
		obs_writeLine(fd, datetime2SNP(T), Ltsys, 'vlbi_initiate_tsys()')

		#T = T + datetime.timedelta(seconds=Ltsys+5)
		#obs_writeLine(fd, datetime2SNP(T), Lmeters, 'readMeters()')
		#T = T + datetime.timedelta(seconds=Lmeters)

		if (Tstart_next != None):
			Lscangap = (Tstart_next - T)
			Lscangap = Lscangap.total_seconds()
			if (Lscangap >= L_minimum_for_interactive):
				msg = 'About %d seconds available for pointing/focusing/other' % (int(Lscangap)) 
				obs_writeLine(fd, datetime2SNP(T), 0, 'interactive(\'%s\')' % (msg))
			fd.write('#     %d seconds (%.1f minutes) until next scan\n\n' % (int(Lscangap),Lscangap/60.0) )

	obs_writeLine(fd, datetime2SNP(T), 1, 'remote_control(\'off\')')


def run(args):
	global src_file, lin_file

	if (len(args) != 3):
		usage()
		sys.exit(-1)

	vexfile = args[1]
	site = args[2]

	sources = getAllSources(vexfile)
	scans = getAllScans(site, vexfile)

	obsfile = vexfile.replace('.vex','') + '.apecs.obs'

	print ('Creating .obs file for site %s' % (site))

	fd = open(obsfile, 'w')
	obs_writeHeader(fd,site,vexfile)
	obs_writeStandardsetup(fd)
	obs_writeScans(fd,scans,sources)
	obs_writeFooter(fd)
	fd.close()
	print ('Wrote obs file       : %s' % (obsfile))

run(sys.argv)
