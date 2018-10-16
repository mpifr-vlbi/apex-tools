FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
CFLAGS = -m32  
LIBS = ../clib/clib.a ../rtelb/rtelb.a
OBJECTS = 

../bin/run : run.o $(OBJECTS)
	cc -o ../bin/run run.o $(CFLAGS) $(OBJECTS) $(LIBS)
	strip ../bin/run
