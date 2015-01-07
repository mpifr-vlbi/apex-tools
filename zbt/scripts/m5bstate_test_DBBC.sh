rm -f m5bstate_test.vdif

# For R2DBE, 1 ch x 2048 MHz mode
#vdifsnapshotUDP --cpu=2 --offset=8 64 4001 m5bstate_test.vdif
#m5bstate m5bstate_test.vdif VDIF_8192-8192-1-2  5000

# For DBBC, 16 x 62.5 MHz mode
vdifsnapshotUDP --cpu=2 --offset=8 64 46227 m5bstate_test.vdif
m5bstate m5bstate_test.vdif VDIF_8000-4000-16-2 500

#modifyVDIF --extract=0 m5bstate_test.vdif m5bstate_test_t0.vdif
#modifyVDIF --extract=1 m5bstate_test.vdif m5bstate_test_t1.vdif
#m5bstate m5bstate_test_t0.vdif VDIF_8000-4000-16-2  500
#m5bstate m5bstate_test_t1.vdif VDIF_8000-4000-16-2  500

