FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
#
CFLAGS= -m32  
LIBES = ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a -lm
#
OBJECTS = antcn.o 
#
../bin/antcn: $(OBJECTS)
	cc  -o ../bin/antcn $(CFLAGS) $(OBJECTS) $(LIBES)
	strip ../bin/antcn
#
antcn:
	rm -f ../bin/antcn
	make
