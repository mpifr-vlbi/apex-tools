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

