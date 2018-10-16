CFLAGS = -m32
FFLAGS= -m32
#
#NIX= 
LIBES = /usr2/fs/poclb/poclb.a /usr2/fs/clib/clib.a /usr2/fs/rtelb/rtelb.a 
#
OBJECTS = dbbccn.o
#
../bin/dbbccn: $(OBJECTS)
	cc -m32  $(CFLAGS) -o ../bin/dbbccn $(OBJECTS) $(LIBES)
	strip ../bin/dbbccn
#
mk5cn:
	rm -f ../bin/dbbccn
	make
