CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
#
OBJECTS = error.o dppco.o dppfa.o dppin.o dppsl.o fecon.o \
fit2.o incne.o incsm.o inine.o inism.o kgetm.o kglin.o kgpnt.o \
kinit.o kpcon.o kpdat.o kpfit.o kplin.o kpout.o kpst.o kptri.o \
rstat.o scaler.o unget.o unscaler.o kpout_ch.o kpdat2.o apost.o indx.o
#
FFLAGS = -m32  -c 
LIBES = ../port/port.a ../polb/polb.a ../fslb/fslb.a ../lnfch/lnfch.a \
../newlb/newlb.a ../flib/flib.a ../fclib/fclib.a ../clib/clib.a \
../fmpsee/fmpsee.a ../vis/vis.a ../rtelb/rtelb.a
#
../bin/error: $(OBJECTS)
	$(FC) -Wl,-m32 -o ../bin/error $(OBJECTS) $(LIBES)
	strip ../bin/error
#
error:
	rm -f ../bin/error
	make
