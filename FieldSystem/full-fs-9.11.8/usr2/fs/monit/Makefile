FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
all: ../bin/monit1 ../bin/monit2 ../bin/monit3 ../bin/monit4 ../bin/monit5
#
FFLAGS = -m32  -c
CFLAGS = -m32   -I/usr/include/ncurses
#LIBES= ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a  -lcurses -ltermlib
LIBES= ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a  -lncurses
#
OBJECTS1 = monit1.o die.o
#
../bin/monit1: $(OBJECTS1)
	cc -o ../bin/monit1 $(CFLAGS) $(OBJECTS1) $(LIBES)
	strip ../bin/monit1
#
OBJECTS2 = monit2.o die.o azel.o sider.o pre_str.o m2init.o\
mout2.o satpos.o
#
../bin/monit2: $(OBJECTS2)
	cc -o ../bin/monit2 $(CFLAGS) $(OBJECTS2) $(LIBES) -lm
	strip ../bin/monit2
#
OBJECTS3 = monit3.o die.o pre_str.o
#
../bin/monit3: $(OBJECTS3)
	cc -o ../bin/monit3 $(CFLAGS) $(OBJECTS3) $(LIBES)
	strip ../bin/monit3
#
OBJECTS4 = monit4.o die4.o m4init.o mout4.o
#
../bin/monit4: $(OBJECTS4)
	cc -o ../bin/monit4 $(CFLAGS) $(OBJECTS4) $(LIBES) -lm
	strip ../bin/monit4
#
OBJECTS5 = monit5.o die.o pre_str.o m5init.o\
mout5.o
#
../bin/monit5: $(OBJECTS5)
	cc -o ../bin/monit5 $(CFLAGS) $(OBJECTS5) $(LIBES) -lm
	strip ../bin/monit5
