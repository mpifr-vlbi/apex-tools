FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
#
CFLAGS= -m32  -g
LIBES = ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a
#
OBJECTS = erchk.o get_err.o
#
../bin/erchk: $(OBJECTS)
	cc -g -o ../bin/erchk $(CFLAGS) $(OBJECTS) $(LIBES) -lm
	strip ../bin/erchk
#
errchk:
	rm -f ../bin/erchk
	make
