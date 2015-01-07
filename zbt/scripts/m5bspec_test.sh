# For R2DBE, 1 ch x 2048 MHz mode
# vdifsnapshotUDP --cpu=2 --offset=8 64 4001 m5spec_test.vdif
# m5spec -nopol m5spec_test.vdif VDIF_8192-8192-1-2 8192 1000 m5spec_test.m5spec

# For DBBC, 16 x 62.5 MHz mode
rm -f m5spec_test.vdif m5spec_test_t0.vdif m5spec_test_t1.vdif
vdifsnapshotUDP --cpu=2 --offset=8 64 46227 m5spec_test.vdif
modifyVDIF --extract=0 m5spec_test.vdif m5spec_test_t0.vdif
modifyVDIF --extract=1 m5spec_test.vdif m5spec_test_t1.vdif
m5spec -nopol m5spec_test_t0.vdif VDIF_8000-4000-16-2 8192 1000 m5spec_test_t0.m5spec
m5spec -nopol m5spec_test_t1.vdif VDIF_8000-4000-16-2 8192 1000 m5spec_test_t1.m5spec

echo "m5specGlueplot('m5spec_test_t0.m5spec')"> ./m5spec_test.m
octave ./m5spec_test.m
rm ./m5spec_test.m


echo "m5specGlueplot('m5spec_test_t1.m5spec')"> ./m5spec_test.m
octave ./m5spec_test.m
rm ./m5spec_test.m

