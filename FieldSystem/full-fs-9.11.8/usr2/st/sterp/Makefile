CFLAGS= -m32
FFLAGS= -m32
#
#NIX= -g
LIBES = ../stlib/stlib.a ../../fs/poclb/poclb.a ../../fs/clib/clib.a\
../../fs/rtelb/rtelb.a
#
OBJECTS = sterp.o get_err.o
#
../bin/sterp: $(OBJECTS)
	cc -m32 -g -o ../bin/sterp $(CFLAGS) $(OBJECTS) $(LIBES) -lm
	strip ../bin/sterp
#
sterp:
	rm -f ../bin/sterp
	make
