FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
LIBES = ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a
#
../bin/pcald: pcald.o $(OBJECTS)
	cc  -o ../bin/pcald pcald.o $(CFLAGS) $(OBJECTS) $(LIBES)
	strip ../bin/pcald

pcald:
	rm -f ../bin/pcald
	make
#
