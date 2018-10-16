FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
LIBES = ../clib/clib.a
OBJECTS= get_rxgain_files.o get_gain_par2.o
#
../bin/gndat: gndat.o $(OBJECTS)
	cc  -o ../bin/gndat gndat.o $(CFLAGS) $(OBJECTS) $(LIBES) -lm
	strip ../bin/gndat
	ln -sfn ../bin/gndat ../bin/gndat2
gndat:
	rm -f ../bin/gndat ../bin/gndat2
	make
#
