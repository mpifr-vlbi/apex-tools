FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
OBJECTS = fmset.o asktime.o get4time.o getfmtime.o getvtime.o\
initvstr.o set4time.o setfmtime.o setvtime.o gets2time.o sets2time.o asksure.o\
get5btime.o set5btime.o getfila10gtime.o setfila10gtime.o
#
CFLAGS = -m32   -I/usr/include/ncurses
LIBES	= ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a
#
../bin/fmset: $(OBJECTS)
	cc  -o ../bin/fmset $(CFLAGS) $(OBJECTS) $(LIBES) -lm -lncurses
	strip ../bin/fmset
#
fmset:
	rm -f ../bin/fmset
	make
