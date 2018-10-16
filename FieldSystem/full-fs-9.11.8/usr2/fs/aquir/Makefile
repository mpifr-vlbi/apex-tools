CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
#
OBJECTS = aquir.o antcn.o delne.o demas.o \
depar.o kgetc.o kinit.o knup.o onsor.o \
scmd.o sctl.o ssrc.o
#
LIBES = ../polb/polb.a ../fslb/fslb.a ../lnfch/lnfch.a ../newlb/newlb.a\
../flib/flib.a ../fclib/fclib.a ../clib/clib.a ../fmpsee/fmpsee.a\
../rtelb/rtelb.a
#
../bin/aquir: $(OBJECTS)
	$(FC) -Wl,-m32 -o ../bin/aquir $(OBJECTS) $(LIBES)
	strip ../bin/aquir
#
aquir:
	rm -f ../bin/aquir
	make
