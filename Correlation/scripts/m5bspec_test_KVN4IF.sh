#!/bin/bash

# For KVN 4-band setup
# FILA10G configured to combine four single-IF VSI buses
# into multi-IF VSI stream

FMT=VDIF_8192-8192-4-2

rm -f m5spec_test.vdif m5spec_test.m5spec
vdifsnapshotUDP --cpu=2 --offset=8 128 46227 m5spec_test.vdif
m5spec -nopol m5spec_test.vdif $FMT 32768 2000 m5spec_test.m5spec
echo "m5specGlueplot('m5spec_test.m5spec')"> ./m5spec_test_KVN.m
octave ./m5spec_test_KVN.m
rm ./m5spec_test_KVN.m

gv m5spec_test.ps &

# Time series plot
m5d m5spec_test.vdif $FMT 8000 0 > m5spec_test.m5d
tail -n +17 m5spec_test.m5d  > tmp  # deletes the "Mark5 stream: 0x13f5140" etc header printout done by m5d
mv tmp m5spec_test.m5d

# gnuplot

#plot "m5spec_test.m5d" u ($1+0) w l t 'VSI 1', \
# "m5spec_test.m5d" u ($2-10) w l t 'VSI 2', \
# "m5spec_test.m5d" u ($3-20) w l t 'VSI 3', \
# "m5spec_test.m5d" u ($4-30) w l t 'VSI 4'
#set yrange [-35:5]; unset ylabel; unset ytics;
#set xrange [0:600]; set key outside; replot
