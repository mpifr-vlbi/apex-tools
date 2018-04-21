CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
OBJECTS = headp.o
#
#NIX = -c  -i2
FFLAGS = -m32  -c 
LIBES = ../fslb/fslb.a\
../lnfch/lnfch.a\
../newlb/newlb.a\
../flib/flib.a\
../fclib/fclib.a\
../clib/clib.a\
../fmpsee/fmpsee.a\
../rtelb/rtelb.a

#
../bin/headp: $(OBJECTS)
	$(FC) -Wl,-m32 -o ../bin/headp $(OBJECTS) $(LIBES)
	strip ../bin/headp
	rm -f ../bin/headp1
	ln -s ../bin/headp ../bin/headp1
	rm -f ../bin/headp2
	ln -s ../bin/headp ../bin/headp2
