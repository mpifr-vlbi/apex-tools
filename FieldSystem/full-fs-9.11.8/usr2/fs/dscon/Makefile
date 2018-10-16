FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
#
CFLAGS= -m32 
LIBES = ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a
#
OBJECTS =\
dscon.o
#
../bin/dscon: $(OBJECTS) $(LIBES)
	cc -o ../bin/dscon $(CFLAGS) $(OBJECTS) $(LIBES) -lm
	strip ../bin/dscon
#
dscon:
	rm -f ../bin/dscon
	make
