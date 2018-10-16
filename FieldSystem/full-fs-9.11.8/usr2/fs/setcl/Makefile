CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
OBJECTS = setcl.o
#
FFLAGS = -m32  -c 
LIBES = ../fslb/fslb.a ../lnfch/lnfch.a ../newlb/newlb.a\
../flib/flib.a ../fclib/fclib.a ../clib/clib.a ../poclb/poclb.a\
../clib/clib.a ../fmpsee/fmpsee.a ../rtelb/rtelb.a
#
../bin/setcl: $(OBJECTS)
	$(FC) -Wl,-m32 -o ../bin/setcl $(OBJECTS) $(LIBES)
	strip ../bin/setcl
#
setcl:
	rm -f ../bin/setcl
	make
