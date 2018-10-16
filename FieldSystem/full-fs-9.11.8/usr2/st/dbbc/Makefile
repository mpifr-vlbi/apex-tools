CFLAGS= -m32
FFLAGS= -m32
# Makefile for library routines
#
#
OBJECTS = dbbcinit.o dbbcrelink.o dbbc_dis.o dbbcclose.o dbbc_direct.o

dbbclib.a: $(OBJECTS)
	touch dbbclib.a
	rm dbbclib.a
	ar -qc dbbclib.a $(OBJECTS)
	ar -s dbbclib.a
