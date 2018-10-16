FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
#LIBS = ../port/port.a -lf2c -lgpib -lfl
LIBS = ../port/port.a
OBJECTS = chk_bauds.o
#
../bin/chk_bauds: chk_bauds.o $(OBJECTS)
	gcc -o ../bin/chk_bauds $(CFLAGS) $(OBJECTS) $(LIBS)
	strip ../bin/chk_bauds
#
chk_bauds:
	rm -f ../bin/chk_bauds
	make
