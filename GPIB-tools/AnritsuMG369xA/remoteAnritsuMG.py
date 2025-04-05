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
		# PDF p. 104
		# return self.query("F5 %d MH ACW" % (f_MHz))
		# PDF p. 119
		cmd = "CF0 %.6f MH" % (f_MHz)
		if self.verbose:
			print("Sending cmd: %s" % (cmd))
		return self.write(cmd)

	def getFrequency(self):
		qry = "OF0"
		if self.verbose:
			print("Sending query: %s" % (qry))
		return self.query(qry)

	def setPower(self, dbm):
		# PDF p.156
		if dbm <= 10:
			self.write("L0<value><unit>") # todo

anritsu = AnritsuMG369xA(host=converter_ip, address=gpib_addr)
anritsu.open(verbose=False)

f0_MHz = 422.990015
print('Current CW freq: ', anritsu.getFrequency()) 
print('Setting CW freq to %.9f' % (f0_MHz))
anritsu.setFrequency(f0_MHz) 
print('Current CW freq: ', anritsu.getFrequency()) 

anritsu.close()
