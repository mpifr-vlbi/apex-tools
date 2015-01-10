#!/usr/bin/python
import sys
from vdif import VDIFFrameHeader, VDIFFrame

VDIF_HDR_LEN = 32
Narg = len(sys.argv) - 1

def usage():
	print ''
	print 'Usage:  vdifTimealignFileset.py <file1.vdif> <file1 Mbit/s> ... <fileN.vdif> <fileN Mbit/s>'
	print ''
	print 'Returns the byte offsets required to approximately align the starting times of the files.'
     	print 'At least 2 files must be specified, together with their respective "sample rates" in Mbit/s.'
	print ''

Nfiles = Narg/2
if (Narg==0) or (Narg%2 == 1) or (Nfiles <= 2):
	usage()
	sys.exit(-1)

files = []

tstart_latest = 0.0

for n in range(Nfiles):
	fname = sys.argv[1+2*n+0]
	fMbps = int(sys.argv[1+2*n+1])
	fid = open(fname, 'rb')
	hdr = VDIFFrameHeader.from_bin(fid.read(VDIF_HDR_LEN))
	fid.close()

	payload_size = hdr.frame_length*8 - VDIF_HDR_LEN
	fps = fMbps*1e6/(8*payload_size)

	tfile = hdr.secs_since_epoch + hdr.data_frame/fps
	if tfile>tstart_latest:
		tstart_latest = tfile

	print 'File %s : %d Mbps : %d-byte payload : %d frames/sec' % (fname,fMbps,payload_size,fps)
	print '          timestamp %d sec + %d/%d frames = %.6f sec' % (hdr.secs_since_epoch,hdr.data_frame,fps,tfile)

	fileinfos = {'framesize':hdr.frame_length*8, 'fps':fps, 'starttime':tfile, 'filename':fname}
	files.append(fileinfos)

print 'File offsets to reach common start time of approx. %.6f seconds:' % (tstart_latest)

for n in range(Nfiles):
	fi = files[n]
	tdiff = tstart_latest - fi['starttime']
	pdiff = int(tdiff * fi['fps'])
	bdiff = pdiff * fi['framesize']
	print 'File %s : dT=%.12f sec : dFrames=%d : offset=%d' % (fi['filename'],tdiff,pdiff,bdiff)

