FFLAGS= -m32
CFLAGS = -m32
#
#NIX= -g
LIBES = ../stlib/stlib.a  \
  ../dbbc/dbbclib.a ../../fs/port/port.a ../../fs/poclb/poclb.a \
  ../../fs/clib/clib.a ../../fs/rtelb/rtelb.a 
#  ../../fs/clib/clib.a ../../fs/rtelb/rtelb.a /usr/lib/libm.a
#
OBJECTS =\
stqkr.o  
#
../bin/stqkr: $(OBJECTS)
	cc $(CFLAGS) -g -o ../bin/stqkr $(OBJECTS) $(LIBES) -lm
	strip ../bin/stqkr
#
stqkr:
	rm -f ../bin/stqkr
	make
