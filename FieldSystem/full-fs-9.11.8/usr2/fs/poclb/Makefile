CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
# Makefile for library routines
#
OBJECTS = pname.o logit.o iadt.o equn.o\
gmodl.o pmdlq.o sider.o refrw.o cnvrt.o limit.o flt.o fln.o echoe.o logita.o\
logite.o novas.o novascon.o sider2.o solsys3.o equn2.o logitf.o logitn.o\
refrwn.o
#
poclb.a: $(OBJECTS)
	touch poclb.a
	rm poclb.a
	ar -qc poclb.a $(OBJECTS)
	ar s poclb.a
