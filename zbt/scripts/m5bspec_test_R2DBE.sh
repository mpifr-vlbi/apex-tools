# For R2DBE, 1 ch x 2048 MHz mode

rm -f m5spec_test.vdif m5spec_test.m5spec m5spec_test.ps
vdifsnapshotUDP --cpu=2 --offset=8 128 4001 m5spec_test.vdif

# m5spec -nopol m5spec_test.vdif VDIF_8192-8192-1-2 16384 1000 m5spec_test.m5spec  # produces wrong frequency axis, mark5acces prob!
m5specR2DBE m5spec_test.vdif xxx 16384 1000 m5spec_test.m5spec

echo "m5specGlueplot('m5spec_test.m5spec')"> ./m5spec_test.m
octave ./m5spec_test.m
rm ./m5spec_test.m
gv m5spec_test.ps

