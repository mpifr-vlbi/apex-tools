FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
LIBES = ../poclb/poclb.a  ../clib/clib.a ../rtelb/rtelb.a
#
../bin/flagr: flagr.o $(OBJECTS)
	cc  -o ../bin/flagr flagr.o $(CFLAGS) $(OBJECTS) $(LIBES)
	strip ../bin/flagr

flagr:
	rm -f ../bin/flagr
	make
#
