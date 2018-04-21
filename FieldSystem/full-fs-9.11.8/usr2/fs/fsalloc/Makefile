FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
# set environment variable NO_FTOK_FS non-empty if ftok() is not available
# if you use csh/tcsh add this to your .login file:
#   setenv NO_FTOK_FS TRUE
# and log-out and back in
#
ifdef NO_FTOK_FS
CFLAGS :=  $(CFLAGS) -DNO_FTOK_FS
endif
LIBS = ../flib/flib.a ../fclib/fclib.a ../clib/clib.a ../rtelb/rtelb.a
OBJECTS = 

../bin/fsalloc : fsalloc.o $(OBJECTS)
	cc -o ../bin/fsalloc fsalloc.o $(CFLAGS) $(OBJECTS) $(LIBS)
	strip ../bin/fsalloc
#
fsalloc:
	rm -f ../bin/fsalloc
	make
