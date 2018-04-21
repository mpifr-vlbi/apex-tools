CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
OBJECTS = pcalr.o bpset.o bsplt.o dbcom.o dprox.o \
dump.o messg.o modet.o nrmlz.o pcalx.o pccor.o tpset.o\
i1bit.o sqish.o tabgn.o
#
#NIX = -c  -i2
FFLAGS = -m32  -c 
LIBES =  ../lnfch/lnfch.a ../fslb/fslb.a ../newlb/newlb.a ../flib/flib.a\
../fclib/fclib.a ../clib/clib.a ../port/port.a ../fmpsee/fmpsee.a\
../rtelb/rtelb.a
#
../bin/pcalr: $(OBJECTS)
	$(FC) -Wl,-m32 -o ../bin/pcalr $(OBJECTS) $(LIBES)
	strip ../bin/pcalr
#
pcalr:
	rm -f ../bin/pcalr
	make
