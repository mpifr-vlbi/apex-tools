FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
OBJECTS = fserr.o listinit.o hashcode.o
#
LIBES =  ../clib/clib.a ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a
#
../bin/fserr: $(OBJECTS)
	cc -o ../bin/fserr $(CFLAGS) $(OBJECTS) $(LIBES)
	strip ../bin/fserr
