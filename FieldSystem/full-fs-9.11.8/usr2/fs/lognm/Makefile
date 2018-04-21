FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
CFLAGS = -m32  
LIBS = ../clib/clib.a ../rtelb/rtelb.a
OBJECTS = 

../bin/lognm : lognm.o $(OBJECTS)
	cc -o ../bin/lognm lognm.o $(CFLAGS) $(OBJECTS) $(LIBS)
	strip ../bin/lognm
