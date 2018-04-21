FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
CFLAGS = -m32  
LIBS = ../clib/clib.a ../rtelb/rtelb.a
OBJECTS = 

../bin/go : go.o $(OBJECTS)
	cc -o ../bin/go go.o $(CFLAGS) $(OBJECTS) $(LIBS)
	strip ../bin/go
