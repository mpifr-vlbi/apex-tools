#!/usr/bin/python
"""
Usage: quickspecR2DBE.py [num of time integrations (int)] [<r2dbe hostname>]

Grabs ADC snapshots, time integrates them, and plots
the spectrum of each input IF.
"""

import adc5g, corr
from pylab import plot, show, title, xlabel, ylabel, subplot, gcf, xlim, semilogy
import pylab
import numpy
import sys, socket

try:
    import matplotlib as mpl
    # Must disable path simplifcation to allow fringe peaks to be seen even in dense plots
    # http://stackoverflow.com/questions/15795720/matplotlib-major-display-issue-with-dense-data-sets
    mpl.rcParams['path.simplify'] = False
except:
    pass

def isInt(s):
    try: 
        int(s)
        return True
    except ValueError:
        return False

def plotSpectrum(y,Fs,tstr):
	"""
	Plots a Single-Sided Amplitude Spectrum of y(t)
	"""
	doLogscale = True

	n = len(y) # length of the signal
	k = numpy.arange(n)
	T = n/Fs
	frq = 1e-6 * k/T # two sides frequency range
	frq = frq[range(n/2)] # one side frequency range

	Y = numpy.fft.fft(y)/n # fft computing and normalization
	Y = Y[range(n/2)]
 
	if doLogscale:
		semilogy(frq,abs(Y),'k')
	else:
		plot(frq,abs(Y),'k')
	xlim([0,frq[-1]])	

	xlabel('Freq (MHz)')
	if doLogscale:
		ylabel('log|Y(freq)|')
	else:
		ylabel('|Y(freq)|')
	title(tstr)

# Defaults
r2dbe_hostname = 'r2dbe-1'
Fs = 2*2048e6 # R2DBE sampling freq
Nif    = 2  # R2DBE typically 2 IFs
Ninteg = 1  # integrate this many ADC snapshots

# Args
args = sys.argv[1:]
while len(args)>0:
	if isInt(args[0]):
		Ninteg = int(args[0])
	else:
		r2dbe_hostname = args[0]
	args = args[1:]

# Check if we can resolve the host
hip = ''
try:
	hip = socket.gethostbyname(r2dbe_hostname)
except:
	pass
if len(hip) < 4:
	print('Error: could not get host %s' % (r2dbe_hostname))
	sys.exit(0)

# Connect
print 'Connecting...'
roach2 = corr.katcp_wrapper.FpgaClient(r2dbe_hostname)
roach2.wait_connected()

Nsamp = [0]*Nif
Lfft  = [0]*Nif
data  = [None]*Nif

for ii in range(Ninteg):
	for ifnr in range(Nif):
	        data8 = adc5g.get_snapshot(roach2, 'r2dbe_snap_8bit_%d_data' % (ifnr))
		Lfft[ifnr] = len(data8)
		Nsamp[ifnr] = Nsamp[ifnr] + Lfft[ifnr]
		if data[ifnr]==None:
			data[ifnr] = data8
		else:
			# data[ifnr] = data[ifnr] + data8
			data[ifnr] = [data[ifnr][n]+data8[n] for n in range(len(data8))]
	        print '   Int %d/%d : ADC %d snapshot of 8-bit data, got %d samples' % (ii+1,Ninteg,ifnr,len(data8))

for ifnr in range(Nif):
	subplot(Nif,1,(ifnr+1))
	plotSpectrum(data[ifnr], Fs, 'ADC %d' % (ifnr))
gcf().set_facecolor('white')
show()
