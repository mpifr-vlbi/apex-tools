# Helper for apexVLBI.py

import struct
import code
from time import time
from socket import socket, AF_INET, SOCK_DGRAM
from datetime import datetime

# Use NTP server for current time because APECS systems run 
# in a quite strange time standard
ntpserver = 'nist1-lnk.binary.net'

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
def get_UTC_offset(ntpserver):
	print 'Contacting NTP server %s to determine local offset to UTC (APECS: TAI-UTC offset)...' % (ntpserver)
	while True:
		ntpUtcNow = send_ntp_request(ntpserver)
		if not(str(ntpUtcNow).startswith('ERROR')):
			break
		print 'NTP server did not reply, retrying in 5 seconds...'
		sleep(5.0)

	pcUtcNow = datetime.utcnow()

	# Complicated method
	ntpUtcNowTM = ntpUtcNow.timetuple()
	pcUtcNowTM = pcUtcNow.timetuple()
	pcUtcNowDoy = int(pcUtcNowTM.tm_yday)
	ntpUtcNowDoy = int(ntpUtcNowTM.tm_yday)
	pcUtcNowSecofyear = pcUtcNowDoy*24*60*60 + pcUtcNowTM.tm_hour*60*60 + pcUtcNowTM.tm_min*60 + int(pcUtcNowTM.tm_sec)
	ntpUtcNowSecofyear = ntpUtcNowDoy*24*60*60 + ntpUtcNowTM.tm_hour*60*60 + ntpUtcNowTM.tm_min*60 + int(ntpUtcNowTM.tm_sec)
	PC_minus_NTP_seconds = pcUtcNowSecofyear - ntpUtcNowSecofyear
	
	# Simple method
	dT = pcUtcNow - ntpUtcNow
	dT = dT.total_seconds()

	#print 'NTP UTC: %s ' % (str(ntpUtcNow))
	#print 'PC  UTC: %s ' % (str(pcUtcNow))
	#print 'PC leads NTP by: %s seconds or %.2f minutes' % (PC_minus_NTP_seconds, int(PC_minus_NTP_seconds)/60.0)
	#print 'PC leads NTP by: %.2f seconds per total_seconds()' % (dT)
	#m_PC_minus_NTP_offset = PC_minus_NTP_seconds

	return dT

