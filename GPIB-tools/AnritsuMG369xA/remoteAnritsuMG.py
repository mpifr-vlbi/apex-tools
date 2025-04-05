#!/usr/bin/python
'''
Basic control of an Anritsu MG369xA series synthesizer
over GPIB - assuming the Anritsu is connected to the LAN
through a Prologix GPIB-Ethernet converter.

todo:
 - add support for user args to set freq & level, and RF On-Off
 - add support for predefined config loaded from a specified config file
 - ...
'''

# External prologix-gpib-ethernet Python package:
# $  pip install git+git://github.com/nelsond/prologix-gpib-ethernet.git
from plx_gpib_ethernet import PrologixGPIBEthernetDevice

converter_ip = '10.0.2.94'
gpib_addr = 5

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

anritsu = AnritsuMG369xA(host=converter_ip, address=gpib_addr)
anritsu.open(verbose=False)

f0_MHz = 422.990015
print('Current settings: ', anritsu.getActiveSettings())

print('Setting CW freq to %.9f' % (f0_MHz))
anritsu.setFrequency(f0_MHz) 

print('Current settings: ', anritsu.getActiveSettings())

anritsu.close()
