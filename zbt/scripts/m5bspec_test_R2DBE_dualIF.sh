# For R2DBE, 1 ch x 2048 MHz mode

rm -f m5spec_test.vdif m5spec_test_if[0-9].vdif \
	m5spec_test_if[0-9].m5spec \
	m5spec_test_if0.ps m5spec_test_if1.ps

vdifsnapshotUDP --cpu=2 --offset=8 512 4001 m5spec_test.vdif
modifyVDIF --extract=0 m5spec_test.vdif m5spec_test_if0.vdif
modifyVDIF --extract=1 m5spec_test.vdif m5spec_test_if1.vdif

# m5spec -nopol m5spec_test_if0.vdif VDIF_8192-8192-1-2 16384 1000 m5spec_test_if0.m5spec  # produces wrong frequency axis, mark5acces prob!
# m5spec -nopol m5spec_test_if1.vdif VDIF_8192-8192-1-2 16384 1000 m5spec_test_if1.m5spec  # produces wrong frequency axis, mark5acces prob!
m5specR2DBE m5spec_test_if0.vdif xxx 32768 1000 m5spec_test_if0.m5spec
m5specR2DBE m5spec_test_if1.vdif xxx 32768 1000 m5spec_test_if1.m5spec

echo "m5specGlueplot('m5spec_test_if0.m5spec')"> ./m5spec_test_if0.m
octave ./m5spec_test_if0.m
rm ./m5spec_test_if0.m

echo "m5specGlueplot('m5spec_test_if1.m5spec')"> ./m5spec_test_if1.m
octave ./m5spec_test_if1.m
rm ./m5spec_test_if1.m

gv m5spec_test_if0.ps & gv m5spec_test_if1.ps

