FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
CFLAGS = -m32  
LIBS = ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a
OBJECTS = 

../bin/popen : popen.o $(OBJECTS)
	cc -o ../bin/popen popen.o $(CFLAGS) $(OBJECTS) $(LIBS)
	strip ../bin/popen
