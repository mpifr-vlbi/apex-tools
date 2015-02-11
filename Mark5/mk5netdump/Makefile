
SSD      = /usr/local/src/streamstor/linux
INCPATHS = -I${SSD}/include/ -I.
LDPATHS  = -L. -L$(SSD)/driver/lib/ -L$(SSD)/lib/ -L$(SSD)/lib/gcc_v4/
LDFLAGS  = $(LDPATHS) -lc -lrt -lfuse -lpthread -lssapi -lwdapi1011 -lm
CFLAGS   = -O3 -Wall -D_LARGEFILE64_SOURCE=1 $(INCPATHS)
CC = g++

all: mk5netdump

mk5netdump: mk5netdump.c
	$(CC) $(CFLAGS) $@.c $(LDFLAGS) -o $@

clean:
	rm -f *.o mk5netdump

