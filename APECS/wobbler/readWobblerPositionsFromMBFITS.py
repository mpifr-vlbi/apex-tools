#!/usr/bin/env python

"""
Read APEX wobbler positions from MBFITS data and
output with UTC UNIX time stamps.

D. Muders, MPIfR, 4/2021
"""

import sys
import BoaMBFits

if (len(sys.argv) != 3):
    print 'Usage: %s <MBFITS dataset name> <subscan>' % (sys.argv[0])
    sys.exit(0)

ds = BoaMBFits.importDataset(sys.argv[1])

scanTable = ds.getTables(EXTNAME='SCAN-MBFITS')[0]
tai2utc = scanTable.getKeyword('TAI2UTC').getValue()

subscan = int(sys.argv[2])

monTable = ds.getTables(EXTNAME='MONITOR-MBFITS', SUBSNUM=subscan)[0]
monTable.open()
monTable.setSelection('MONPOINT=="WOBDISPL"')
wobTime = (monTable.getColumn('MJD').read() - 40587.0) * 86400.0 - tai2utc
wobPos = monTable.getColumn('MONVALUE').read()
monTable.close()

ds.close()

for i in range(len(wobTime)):
    print '%.3f %.2f' % (wobTime[i], wobPos[i][0]*3600.)

