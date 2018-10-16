FFLAGS= -m32
CFLAGS=-Wall -g -m32
LIBES = ../../fs/poclb/poclb.a ../../st/stlib/stlib.a ../../fs/clib/clib.a\
../../fs/rtelb/rtelb.a -lm -lnsl

OBJECTS = antcn.o rad2str.o  # antcn_rpc_client.o antcn_rpc_clnt.o

../bin/antcn: $(OBJECTS) ../apecs/apecs_iface.c ../apecs/apecs_iface.h
	$(CC) $(CFLAGS) -o ../bin/antcn $(OBJECTS) $(LIBES)

test: test.o ../apecs/apecs_iface.c ../apecs/apecs_iface.h
	 $(CC) $(CFLAGS) -o test test.o

antcn:
	rm -f ../bin/antcn 
	make

install: antcn
	cp ../bin/antcn ../../fs/bin/antcn

clean:
	rm -f ../bin/antcn
	rm -f *.o test

