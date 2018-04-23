#!/usr/bin/python
"""
Poll Mark6 machines and read their 'lmsensors'.
"""

import subprocess, time
from datetime import datetime
from colorama import Fore, Back, Style

hosts = ['oper@recorder1', 'oper@recorder2', 'oper@recorder3', 'oper@recorder4' ]
temperatures = ['Physical', 'Core', 'temp', 'CPUTIN', 'SYSTIN', 'AUXTIN', 'fan2']

def filter(lines,keys):
	out = []
	for l in lines:
		for k in keys:
			N = len(k)
			if l[:N] == k[:N]:
				# reformat
				i = l.find(':')
				if i >= 0:
					l = '%-15s %s' % (l[:i].strip(), l[(i+1):].strip())
				i = l.find(')')
				if i >= 0:
					l = l[:(i+1)]
				out.append(l)
				break
	return out

def readoutMark6s():
	clear = '\033c'
	colcycle = [Fore.GREEN, Fore.MAGENTA, Fore.CYAN, Fore.BLUE]
	iter = 0
	while True:

		# Collect new printouts (slowly) before actually printing
		next_out = []

		# Header
		T = datetime.utcnow()
		title = '%s refresh %d' % (str(T),iter)
		#next_out.append(clear)
		#next_out.append(Back.BLUE + Fore.WHITE + '  %-70s  ' % (title))
		n = 0

		# Readings from hosts
		for h in hosts:

			# Temperatures
			cmd = '/usr/bin/ssh %s /usr/local/bin/sensors' % (h)
			got_sensors = False
			try:
				# Python 2.7+
				#s = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)
				# Python 2.6
				proc = subprocess.Popen(cmd.split(' '), stdout=subprocess.PIPE)
				s = proc.communicate()[0]
				got_sensors = True
			except Exception as e:
				#print (str(e))
				next_out.append(Fore.RED + Back.BLACK + '%-10s : '%(h) + str(e))
			if got_sensors:
				l = s.split('\n')
				fl = filter(l, temperatures)
				for line in fl:
					next_out.append(colcycle[n] + Back.BLACK + '%-10s : '%(h) + Fore.RESET + line)
			# Disks
			cmd = ['/usr/bin/ssh', h, '/bin/mount | /bin/grep disks | /bin/grep -v meta']
			got_disks = False
			try:
				# Python 2.7+
				#s = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)
				# Python 2.6
				proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
				s = proc.communicate()[0]
				got_disks = True
			except Exception as e:
				#print (str(e))
				pass
			disks = []
			if got_disks:
				for l in s.split('\n'):
					# l = '/dev/sdv1 on /mnt/disks/3/4 type xfs (rw)'
					entries = l.split()
					if len(entries) < 2:
						continue
					dev = entries[0][:-1]
					disks.append(dev)
				summary = '%d disks' % (len(disks))
				next_out.append(colcycle[n] + Back.BLACK + '%-10s : '%(h) + Fore.RESET + summary)

			# Disk temperatures
			temps = []
			for disk in disks:
				cmd = ['/usr/bin/ssh', h,  'sudo /usr/sbin/smartctl -A %s | /bin/grep Temperature_Celsius' % (disk)]
				try:
					# Python 2.7+
					#s = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)
					# Python 2.6
					proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
					s = proc.communicate()[0]
				except Exception as e:
					#print (str(e))
					continue
				vals = s.split()
				if len(vals)<9:
					#print (disk, s, vals)
					continue
				temps.append(int(vals[9]))
			if len(disks)>0 and len(temps)>0:
				# summary = 'disk temps ' + ' '.join([str(t) for t in temps])
				M = sum(temps)/len(temps)
				summary = 'disk temperatures %dC min %dC mean %dC max' % (min(temps),M,max(temps))
				next_out.append(colcycle[n] + Back.BLACK + '%-10s : '%(h) + Fore.RESET + summary)

			# Next color code
			n = (n + 1) % len(colcycle)

		# Now print
		for l in next_out:
			print (l)

		time.sleep(5)
		iter = iter + 1


readoutMark6s()

