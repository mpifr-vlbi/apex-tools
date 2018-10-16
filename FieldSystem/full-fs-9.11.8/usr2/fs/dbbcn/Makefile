FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
#
CFLAGS= -m32  
LIBES = ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a
#
OBJECTS = dbbcn.o
#
../bin/dbbcn: $(OBJECTS)
	cc  -o ../bin/dbbcn $(CFLAGS) $(OBJECTS) $(LIBES)
	strip ../bin/dbbcn
#
dbbcn:
	rm -f ../bin/dbbcn
	make
