#!/usr/bin/python
'''
Basic control of an Anritsu MG369xA series synthesizer
over GPIB - assuming the Anritsu is connected to the LAN
through a Prologix GPIB-Ethernet converter.

The remoteAnritsuMG.py <commands> supported commands are
"freq <xxx.x> mhz", "pow <xxx.x> dbm", "output on|off".
'''

# External prologix-gpib-ethernet Python package:
# $  pip install git+git://github.com/nelsond/prologix-gpib-ethernet.git
from plx_gpib_ethernet import PrologixGPIBEthernetDevice

import argparse

class AnritsuMG369xA(PrologixGPIBEthernetDevice):

	def open(self, verbose=True):
		self.verbose = verbose
		self.connect()
		self.reset()
		self.devname = self.idn()
		if self.verbose:
			print("Connected to %s gpib addr %d device ID %s" % (self.gpib.host, self.address, self.devname))

	def setFrequency(self, f_MHz):
		cmd = "CF0 %.6f MH" % (f_MHz) # PDF p. 119 - 'Sets CW mode at F0 and opens the F0 parameter.'
		if self.verbose:
			print("Sending cmd: %s" % (cmd))
		self.write(cmd)
		self.write("CLO")

	def getFrequency(self):
		qry = "OF0"
		if self.verbose:
			print("Sending query: %s" % (qry))
		fq = self.query(qry)
		return float(fq) # TODO: any units to parse? scale?

	def getPower(self):
		self.write("LOG")
		qry = "OL0"
		if self.verbose:
			print("Sending query: %s" % (qry))
		pwr = self.query(qry)
		return float(pwr) # TODO: any units to parse? scale?

	def setPower(self, dbm):
		# PDF p.156
		if dbm <= 10:
			self.write("LOG") # 
			self.write("PU0") # PDF p.208 - 'Selects logarithmic power level operation in dBm.'
			self.write("L0 %.2f DM" % (dbm))
			self.write("CLO")

	def rfOff(self):
		return self.write("RF0") # PDF p.260 - 'Turns off the RF output.'

	def rfOn(self):
		return self.write("RF1") # PDF p.260 - 'Turns on the RF output.'

	def enableRF(self, ena=False):
		if not ena:
			self.rfOff()
		else:
			self.rfOn()

	def getActiveSettings(self):
		f = self.getFrequency()
		p = self.getPower()
		rfstate = '??' # todo
		return "%.9f MHz / %.2f dBm / RF %s" % (f,p,rfstate)


parser = argparse.ArgumentParser(description="Anritsu MG369xA basic GPIB remote control via an attached Prologix GPIB-LAN converter", epilog=__doc__)
parser.add_argument('-H', '--host', dest='host', default='10.0.2.94', help='hostname of the Prologix GPIB-LAN (default %(default)s)')
parser.add_argument('-a', '--gpib-addr', dest='gpib_addr', default='5', help='address of the MG369xA on the GPIB bus (default %(default)s)')
parser.add_argument('cmds', nargs='*', help='commands to send')
args = parser.parse_args()

anritsu = AnritsuMG369xA(host=args.host, address=int(args.gpib_addr))
anritsu.open(verbose=False)
print('Current settings: ', anritsu.getActiveSettings())

while len(args.cmds) > 0:
	cmd = args.cmds.pop(0).lower()
	if cmd == 'freq':
		f = args.cmds.pop(0).lower()
		f_units = args.cmds.pop(0).lower()
		if f_units == 'ghz':
			f = '%.9f' % (float(f) * 1e3)
		elif f_units == 'hz':
			f = '%.9f' % (float(f) * 1e-6)
		anritsu.setFrequency(float(f))
	elif cmd == 'pow':
		p = args.cmds.pop(0).lower()
		p_units = args.cmds.pop(0).lower()
		if p_units in ['dbm','db']:
			anritsu.setPower(float(p))
	elif cmd == 'output':
		on = args.cmds.pop(0).lower() == 'on'
		anritsu.enableRF(on)
	else:
		print("Unsupported command '%s'" % (cmd))

print('Updated settings: ', anritsu.getActiveSettings())

anritsu.close()
