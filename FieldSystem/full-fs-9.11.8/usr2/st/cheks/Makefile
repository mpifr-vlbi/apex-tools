CFLAGS= -m32
FFLAGS= -m32
#
CFLAGS= -g -m32
#
LIBS = -lm ../../fs/poclb/poclb.a ../../fs/clib/clib.a\
../../fs/rtelb/rtelb.a -lm
#
OBJECTS = cheks.o
#
../bin/cheks : $(OBJECTS)
	cc -g -m32 -o ../bin/cheks $(CFLAGS) $(OBJECTS) $(LIBS) -lm
	strip ../bin/cheks
#
cheks : 
	rm -f ../bin/cheks
	make
