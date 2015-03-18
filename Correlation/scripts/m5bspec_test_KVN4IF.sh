# For KVN 4-band setup

# Scenario 1: FILA10G reads from 4 VSI inputs carrying 1-channel 2-bit data,
# and using the "chanperm" register, convers this to produce 4-channel 2-bit VDIF.

rm -f m5spec_test.vdif m5spec_test.m5spec
vdifsnapshotUDP --cpu=2 --offset=8 128 46227 m5spec_test.vdif
m5spec -nopol m5spec_test.vdif VDIF_8192-8192-4-2 32768 2000 m5spec_test.m5spec
echo "m5specGlueplot('m5spec_test.m5spec')"> ./m5spec_test_KVN.m
octave ./m5spec_test_KVN.m
rm ./m5spec_test_KVN.m
gv m5spec_test.ps

# Scenario 2: FILA10G reads from 4 VSI inputs carrying 1-channel 2-bit data,
# and sends each VSI as a 1-channel VDIF Thread (multithreaded VDIF).

# splitmode on
# vdif_frame 16 1 2000 ct=on
# vdif_frame 32 1 2000 ct=on

rm -f t0.vdif t1.vdif t2.vdif t3.vdif t0.m5spec t1.m5spec t2.m5spec t3.m5spec m5spec_test.vdif
vdifsnapshotUDP --cpu=2 --offset=8 128 46227 m5spec_test.vdif
modifyVDIF --extract=0 m5spec_test.vdif t0.vdif
modifyVDIF --extract=1 m5spec_test.vdif t1.vdif
modifyVDIF --extract=2 m5spec_test.vdif t2.vdif
modifyVDIF --extract=3 m5spec_test.vdif t3.vdif
m5spec -nopol t0.vdif VDIF_2048-2048-1-2 32768 2000 t0.m5spec
m5spec -nopol t1.vdif VDIF_2048-2048-1-2 32768 2000 t1.m5spec
m5spec -nopol t2.vdif VDIF_2048-2048-1-2 32768 2000 t2.m5spec
m5spec -nopol t3.vdif VDIF_2048-2048-1-2 32768 2000 t3.m5spec


# Scenario 3: FILA10G just copies all 128 bits without additional
# modifications and outputs them all in a single VDIF stream.
# Then extract individual VSI later using 'shrinkVDIF'.

rm -f m5spec_test.vdif vsi1.vdif vsi2.vdif vsi3.vdif vsi4.vdif vsi1.m5spec vsi2.m5spec vsi3.m5spec vsi4.m5spec
vdifsnapshotUDP --cpu=2 --offset=8 128 46227 m5spec_test.vdif
# Frame size 8192+32 reduces to 2048+32 by taking only one 32-bit VSI interface
# Data rate of 8192 Mbps (4 VSI) reduces to single-VSI rate of 2048 Mbps
shrinkVDIF -x 1 m5spec_test.vdif vsi1.vdif
shrinkVDIF -x 2 m5spec_test.vdif vsi2.vdif
shrinkVDIF -x 3 m5spec_test.vdif vsi3.vdif
shrinkVDIF -x 4 m5spec_test.vdif vsi4.vdif
m5spec -nopol vsi1.vdif VDIF_2048-2048-1-2 8192 2000 vsi1.m5spec
m5spec -nopol vsi2.vdif VDIF_2048-2048-1-2 8192 2000 vsi2.m5spec
m5spec -nopol vsi3.vdif VDIF_2048-2048-1-2 8192 2000 vsi3.m5spec
m5spec -nopol vsi4.vdif VDIF_2048-2048-1-2 8192 2000 vsi4.m5spec
echo "plot \"vsi1.m5spec\" using 1:2 with lines title \"VSI1\", \\" > cmd
echo "     \"vsi2.m5spec\" using 1:2 with lines title \"VSI2\", \\" >> cmd
echo "     \"vsi3.m5spec\" using 1:2 with lines title \"VSI3\", \\" >> cmd
echo "     \"vsi4.m5spec\" using 1:2 with lines title \"VSI4\" " >> cmd
echo "set logscale y; replot; pause -1" >> cmd
gnuplot cmd

