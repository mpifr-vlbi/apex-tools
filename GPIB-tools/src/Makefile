OUTDIR = ../bin/
CFLAGS = -Wall -O0
#CFLAGS += -DVERBOSE  # comment out to disable printouts

BINARIES=pollVectorVoltmeter pollCounters pollMaser preplot

all: $(BINARIES)

clean:
	rm -f *.o $(BINARIES)

pollVectorVoltmeter: pollVectorVoltmeter.o nettools.o
	$(CC) pollVectorVoltmeter.o nettools.o -o pollVectorVoltmeter

pollCounters: pollCounters.o nettools.o
	$(CC) pollCounters.o nettools.o -o pollCounters

pollMaser: pollMaser.o nettools.o
	$(CC) pollMaser.o nettools.o -o pollMaser

preplot: preplot.o

install:
	cp pollVectorVoltmeter $(OUTDIR)/
	cp pollCounters $(OUTDIR)/
	cp pollMaser $(OUTDIR)/
	cp preplot $(OUTDIR)/
