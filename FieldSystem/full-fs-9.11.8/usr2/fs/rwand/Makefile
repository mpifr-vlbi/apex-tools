CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
OBJECTS = rwand.o bsort.o cksum.o wandwr.o
#
#NIX = -c  -i2
FFLAGS = -m32  -c 
LIBES = ../fslb/fslb.a ../lnfch/lnfch.a ../newlb/newlb.a ../port/port.a\
../flib/flib.a ../fclib/fclib.a\
../clib/clib.a ../fmpsee/fmpsee.a ../rtelb/rtelb.a
#
../bin/rwand: $(OBJECTS)
	$(FC) -Wl,-m32 -o ../bin/rwand $(OBJECTS) $(LIBES)
	strip ../bin/rwand
