FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
#
CFLAGS= -m32 
LIBES = ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a
#
ifeq (/usr/src/linux-1.2.12,\
$(wildcard /usr/src/linux-1.2.12 ))
CFLAGS := $(CFLAGS) -DUSE_OLD_SPECIAL_FLAGS
endif
ifeq (/usr/src/linux/include/digi.h,\
$(wildcard /usr/src/linux/include/digi.h ))
CFLAGS := $(CFLAGS) -DDIGI
STRUCT :=$(shell fgrep >/dev/null 2>&1 'struct digi_t ' /usr/src/linux/include/digi.h ; echo $$?)
ifeq ($(STRUCT),0)
CFLAGS := $(CFLAGS) -DUSE_STRUCT
endif
endif
ifdef FS_SERIAL_CLOCAL
CFLAGS := $(CFLAGS) -DFS_SERIAL_CLOCAL
endif
#
OBJECTS = mcbcn.o
#
../bin/mcbcn: $(OBJECTS)
	cc  -o ../bin/mcbcn $(CFLAGS) $(OBJECTS) $(LIBES)
	strip ../bin/mcbcn
#
mcbcn:
	rm -f ../bin/mcbcn
	make
