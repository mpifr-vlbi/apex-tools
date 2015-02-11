#!/bin/bash
#
# Takes two Mark5B input files for zero baseline correlation.
#
# From each file this script extracts short pieces of data
# from the start of the first new second, and the starts of the
# two following seconds.
#
# These three data pieces from each file pair are cross-correlated
# using the 'vlbi2' tool.
#

if [ "$2" == "" ]; then
        echo "Usage: $0 <file1> <file2>"
        exit
fi

export PATH=$PATH:/usr/src/scripts/

# Skip extraction, use already extracted data files -- or make new files?
if [ "$3" == "" ]; then
        LEN=$((2*1024*1024))
        extract_second5Bbytes.py $1 file1_s1.m5b +1 $LEN
        extract_second5Bbytes.py $1 file1_s2.m5b +2 $LEN
        extract_second5Bbytes.py $1 file1_s3.m5b +3 $LEN

        extract_second5Bbytes.py $2 file2_s1.m5b +1 $LEN
        extract_second5Bbytes.py $2 file2_s2.m5b +2 $LEN
        extract_second5Bbytes.py $2 file2_s3.m5b +3 $LEN
fi

# Channel bit statistics
m5bstate file1_s1.m5b Mark5B-512-16-2 400 | tail -17 > file1_s1.bstate
m5bstate file2_s1.m5b Mark5B-512-16-2 400 | tail -17 > file2_s1.bstate

# Actual cross-correlations with a few integer second offsets
XCARGS="-proctime 100 -2bit 1 -tforce 1"
vlbi2 file1_s1.m5b file2_s1.m5b $XCARGS; mv dd1.pos xcorr_s1s1.ps
vlbi2 file1_s2.m5b file2_s2.m5b $XCARGS; mv dd1.pos xcorr_s2s2.ps
vlbi2 file1_s3.m5b file2_s3.m5b $XCARGS; mv dd1.pos xcorr_s3s3.ps

vlbi2 file1_s2.m5b file2_s1.m5b $XCARGS; mv dd1.pos xcorr_s2s1.ps
vlbi2 file1_s2.m5b file2_s3.m5b $XCARGS; mv dd1.pos xcorr_s2s3.ps

vlbi2 file1_s1.m5b file2_s3.m5b $XCARGS; mv dd1.pos xcorr_s1s3.ps

vlbi2 file1_s1.m5b file1_s1.m5b $XCARGS; mv dd1.pos acorr_f1sec1.ps
vlbi2 file2_s1.m5b file2_s1.m5b $XCARGS; mv dd1.pos acorr_f2sec1.ps

