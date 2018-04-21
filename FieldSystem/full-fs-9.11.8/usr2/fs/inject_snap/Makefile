FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
CFLAGS = -m32  
LIBS =  ../clib/clib.a ../rtelb/rtelb.a
OBJECTS = inject_snap.o

../bin/inject_snap : $(OBJECTS)
	cc -o ../bin/inject_snap $(CFLAGS) $(OBJECTS) $(LIBS)
	strip ../bin/inject_snap
#
fs:
	rm -f ../bin/inject_snap
	make
