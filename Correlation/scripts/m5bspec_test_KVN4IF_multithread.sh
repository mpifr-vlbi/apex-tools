# For KVN 4-band setup
# FILA10G configured to combine four single-IF VSI buses
# into multi-IF VSI stream

# multithreaded VDIF
# Gino says this works only for: 
#  eth0 with 2 VDIF Threads (thread 0: VSI1, thread 1: VSI2), separately 2048 Mbps, total 4096 Mbps
#  eth1 with single VDIF thread (thread 0: VSI3 and VSI4), rate 4096 Mbps
# This may not really help KVN tests...

#rm -f t0.vdif t1.vdif t2.vdif t3.vdif t0.m5spec t1.m5spec t2.m5spec t3.m5spec m5spec_test.vdif
#vdifsnapshotUDP --cpu=2 --offset=8 128 46227 m5spec_test.vdif
#modifyVDIF --extract=0 m5spec_test.vdif t0.vdif
#modifyVDIF --extract=1 m5spec_test.vdif t1.vdif
#modifyVDIF --extract=2 m5spec_test.vdif t2.vdif
#modifyVDIF --extract=3 m5spec_test.vdif t3.vdif
#m5spec -nopol t0.vdif VDIF_2048-2048-1-2 32768 2000 t0.m5spec
#m5spec -nopol t1.vdif VDIF_2048-2048-1-2 32768 2000 t1.m5spec
#m5spec -nopol t2.vdif VDIF_4096-2048-1-2 32768 2000 t2.m5spec
#m5spec -nopol t3.vdif VDIF_4096-2048-1-2 32768 2000 t3.m5spec
