CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
OBJECTS = labck.o
#
LIBES = ../fslb/fslb.a ../lnfch/lnfch.a ../newlb/newlb.a
#
../bin/labck: $(OBJECTS)
	$(FC) -Wl,-m32 -o ../bin/labck $(OBJECTS) $(LIBES)
	strip ../bin/labck
#
labck:
	rm -f ../bin/labck
	make
