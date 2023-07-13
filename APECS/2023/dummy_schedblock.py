#!/usr/bin/python

doy=79 

for hh in range(20,24,1):
        for mm in range(0,60,5):
                print("scan %03d-%02d%02d;" % (doy,hh,mm))
                print("     start=2022y%03dd%02dh%02dm00s; mode=1mmlcp; source=3C84;" % (doy,hh,mm))
                print("     station=Aa:    0 sec:   60 sec:    0.000 GB:   :       : 1;")
                print("     station=Ax:    0 sec:   60 sec:    0.000 GB:   :       : 1;")
                print("endscan;")

