FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
LIBES = ../clib/clib.a ../poclb/poclb.a ../rtelb/rtelb.a
#
../bin/tpicd: tpicd.o $(OBJECTS)
	cc  -o ../bin/tpicd tpicd.o $(CFLAGS) $(OBJECTS) $(LIBES) -lm
	strip ../bin/tpicd

tpicd:
	rm -f ../bin/tpicd
	make
#
