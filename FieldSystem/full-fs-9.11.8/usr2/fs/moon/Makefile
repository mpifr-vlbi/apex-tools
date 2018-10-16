CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
OBJECTS = moon.o gd2xy.o moonp.o papvl.o
#
#NIX = -c  -i2
FFLAGS = -m32  -c 
LIBES = ../port/port.a ../polb/polb.a ../fslb/fslb.a ../flib/flib.a\
../lnfch/lnfch.a ../newlb/newlb.a ../fclib/fclib.a ../clib/clib.a \
../fmpsee/fmpsee.a ../vis/vis.a ../rtelb/rtelb.a
#
../bin/moon: $(OBJECTS)
	$(FC) -Wl,-m32 -o ../bin/moon $(OBJECTS) $(LIBES)
	strip ../bin/moon
