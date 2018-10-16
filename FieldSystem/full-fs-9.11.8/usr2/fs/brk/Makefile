FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
CFLAGS = -m32  
LIBS = ../clib/clib.a ../rtelb/rtelb.a
OBJECTS = 

../bin/brk : brk.o $(OBJECTS)
	cc -o ../bin/brk brk.o $(CFLAGS) $(OBJECTS) $(LIBS)
	strip ../bin/brk
