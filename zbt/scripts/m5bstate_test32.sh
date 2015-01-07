# For DBBC, 16 x 32 MHz mode
rm -f m5bstate_test_t0.vdif m5bstate_test_t1.vdif m5bstate_test.vdif
vdifsnapshotUDP --cpu=2 --offset=8 64 46227 m5bstate_test.vdif
modifyVDIF --extract=0 m5bstate_test.vdif m5bstate_test_t0.vdif
modifyVDIF --extract=1 m5bstate_test.vdif m5bstate_test_t1.vdif
m5bstate m5bstate_test_t0.vdif VDIF_8000-2048-16-2  500
m5bstate m5bstate_test_t1.vdif VDIF_8000-2048-16-2  500

