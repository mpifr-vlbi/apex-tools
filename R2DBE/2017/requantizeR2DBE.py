#!/usr/bin/python
"""
Usage: requantizeR2DBE.py [<r2dbe hostname>]

Readjust the quantization thresholds on the R2DBE.
"""
import adc5g, corr
import sys, socket

r2dbe_hostname = 'r2dbe-1'
if len(sys.argv)==2:
    r2dbe_hostname = sys.argv[1]

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

# Set default thresholds
Nif = 2
th = [128] * Nif
print 'Setting quantization levels to %s...' % (str(th))
for ifnr in range(Nif):
	roach2.write_int('r2dbe_quantize_%d_thresh' % (ifnr), th[ifnr])	

# JanW: -7 dBm over 0-1.5 GHz band input to R2DBE in Bonn VLBI lab
#  thresh=16 produced 10%:41%:41%:8%
#  thresh=12 produced 16%:35%:35%:14%

# automatically adjust thresholds using data
print 'Auto-thresholding...'
while True:

	thnew = [0]*Nif
	for ifnr in range(Nif):
		print '   Getting ADC %d snapshot of 8-bit data' % (ifnr)
		x8   = adc5g.get_snapshot(roach2, 'r2dbe_snap_8bit_%d_data' % (ifnr))
		L    = len(x8)
		y    = sorted(x8)
		Lt1  = int(L*0.16)
		th1  = abs(y[Lt1-1])
		Lt2  = int(L*0.84)
		th2  = abs(y[Lt2-1])
		thnew[ifnr] = (th1+th2)/2
		roach2.write_int('r2dbe_quantize_%d_thresh' % (ifnr), thnew[ifnr])
		print '   Auto-thresholding input %d using %d samples : new thresh = %d' % (ifnr,L,thnew[ifnr])

	if thnew == th:
		print 'Converged. Done.'
		break

	print ''
	print 'Thresholds not converged yet (prev: %s, current: %s), repeating' % (str(th),str(thnew))
	th = thnew
