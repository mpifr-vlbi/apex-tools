#!/usr/bin/env python
'''
Usage: vexGetSources2cat.py <vexfile.vex> [<vexfile.vex> ...]

Extract source names and coordinates from VEX file(s)
and show them in APECS .cat format.

(C) 2018 Jan Wagner
'''
import sys

def coordReformat(s):
        s = s.strip()
        repl = list('hmd\'')
        for c in repl:
                s = s.replace(c,':')
        s = s.replace('s','')
        s = s.replace('"','')
        return s

def checkVex(fn):
        currSrc = ''
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

if (len(sys.argv) < 2) or (sys.argv[1] == '--help' or sys.argv[1]=='-h'):
	print (__doc__)
	sys.exit(1)

for fn in sys.argv[1:]:
        checkVex(fn)

