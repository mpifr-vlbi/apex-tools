#!/usr/bin/env python
#
# Print a VEX $SCHED section that covers today
#

import datetime

T = datetime.datetime.utcnow().timetuple()
year = T.tm_year
doy = T.tm_yday
# doy = 84

print("$SCHED;")
for hh in range(0,24,1):
        for mm in range(0,60,5):
                print("scan %03d-%02d%02d;" % (doy,hh,mm))
                print("     start=%dy%03dd%02dh%02dm00s; mode=1mmlcp; source=3C84;" % (year,doy,hh,mm))
                print("     station=Aa:    0 sec:   60 sec:    0.000 GB:   :       : 1;")
                print("     station=Ax:    0 sec:   60 sec:    0.000 GB:   :       : 1;")
                print("endscan;")

