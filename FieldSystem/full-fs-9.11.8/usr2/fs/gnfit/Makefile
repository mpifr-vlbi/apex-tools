CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
LIBES = ../vis/vis.a ../clib/clib.a
OBJECTS= gnfit.o fit2.o  fpoly.o  scaler.o unscaler.o inine.o incne.o \
dppco.o dppfa.o dppin.o dppsl.o ftau.o
#
FFLAGS = -m32  -c 
../bin/gnfit: $(OBJECTS)
	$(FC) -m32  -o ../bin/gnfit $(OBJECTS) $(LIBES)
	strip ../bin/gnfit

gnfit:
	rm -f ../bin/gnfit
	make
#
