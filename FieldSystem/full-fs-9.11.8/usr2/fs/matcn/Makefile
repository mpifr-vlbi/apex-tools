CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
OBJECTS = matcn.o datat.o echoe.o iat.o
#
#NIX = -c  -i2
FFLAGS = -m32  -c 
LIBES = ../port/port.a ../fslb/fslb.a ../lnfch/lnfch.a  \
../flib/flib.a ../fclib/fclib.a ../clib/clib.a ../newlb/newlb.a\
../rtelb/rtelb.a
#
../bin/matcn: $(OBJECTS)
	$(FC) -Wl,-m32 -o ../bin/matcn $(OBJECTS) $(LIBES)
	strip ../bin/matcn
#
matcn:
	rm -f ../bin/matcn
	make
