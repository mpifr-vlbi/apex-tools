CFLAGS= -m32
FFLAGS= -m32
#NIX =-g
LIBS = ../stlib/stlib.a
#
../bin/stalloc : stalloc.o $(OBJECTS)
	cc -m32 -o ../bin/stalloc stalloc.o $(CFLAGS) $(OBJECTS) $(LIBS)
	strip ../bin/stalloc
