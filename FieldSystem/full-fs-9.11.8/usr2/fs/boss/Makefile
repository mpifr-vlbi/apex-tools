CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
#
all: ../bin/boss ../bin/helpsh
#
LIBES = ../bosslb/bosslb.a ../lnfch/lnfch.a ../fslb/fslb.a ../newlb/newlb.a \
../flib/flib.a ../fclib/fclib.a ../clib/clib.a ../fmpsee/fmpsee.a\
../rtelb/rtelb.a
#
OBJECTS = boss.o binit.o bwork.o cants.o helpstr.o fshelp.o\
getcm.o getts.o lists.o newpf.o newpr.o \
newsk.o opnpf.o putts.o rdtib.o rdtma.o \
spars.o stat.o newlg.o ldrivev.o ldrivem.o ldriveall.o lhead.o\
follow_link.o
#
../bin/boss: $(OBJECTS)
	$(FC) -Wl,-m32 -o ../bin/boss $(OBJECTS) $(LIBES)
	strip ../bin/boss
#
../bin/helpsh: helpsh
	cp helpsh ../bin/helpsh
	chmod a+x ../bin/helpsh

boss:
	rm -f ../bin/boss
	make
