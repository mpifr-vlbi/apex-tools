CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
OBJECTS = chekr.o bbc_brk.o bbchk.o dist_brk.o distchk.o\
err_rep.o getfm.o getif.o gettp.o getvc.o\
hdchk.o mk3drive.o mk3rack.o rec_brk.o recchk.o\
rxchk.o vform_brk.o vformchk.o vlbadrive.o vlbarack.o timechk.o i3chk.o\
s2recstatus.o s2drive.o s2recchk.o lbarack.o ifpchk.o daschk.o ifpstatus.o\
s2daschk.o
#
#NIX = -c  -i2
FFLAGS = -m32  -c 
LIBES = ../fslb/fslb.a ../lnfch/lnfch.a ../flib/flib.a ../fclib/fclib.a \
../poclb/poclb.a ../clib/clib.a ../newlb/newlb.a ../rtelb/rtelb.a \
../s2das/s2das.a
#
../bin/chekr: $(OBJECTS)
	$(FC) -Wl,-m32 -o ../bin/chekr $(OBJECTS) $(LIBES)
	strip ../bin/chekr
#
chekr:
	rm -f ../bin/chekr
	make
