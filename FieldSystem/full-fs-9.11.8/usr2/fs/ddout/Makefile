FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
LIBES = ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a
OBJECTS =\
recover_log.o
#
../bin/ddout: ddout.o $(OBJECTS)
	cc  -o ../bin/ddout ddout.o $(CFLAGS) $(OBJECTS) $(LIBES)
	strip ../bin/ddout

ddout:
	rm -f ../bin/ddout
	make
#
