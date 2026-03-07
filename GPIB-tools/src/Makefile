OUTDIR = ../bin/
CFLAGS = -Wall -O0
CXXFLAGS = -Wall -O0
#CFLAGS += -DVERBOSE  # comment out to disable printouts

BINARIES=pollVectorVoltmeter pollCounters pollMaser pollCounters_maser pollCounters_both initBothCounters queryBothCounters

all: $(BINARIES)

clean:
	rm -f *.o $(BINARIES)

initBothCounters: HPCounterGPIB.o PrologixEthGPIB.o initBothCounters.o
	$(CXX)  HPCounterGPIB.o PrologixEthGPIB.o initBothCounters.o -o initBothCounters

queryBothCounters: HPCounterGPIB.o PrologixEthGPIB.o queryBothCounters.o
	$(CXX)  HPCounterGPIB.o PrologixEthGPIB.o queryBothCounters.o -o queryBothCounters

pollVectorVoltmeter: pollVectorVoltmeter.o nettools.o
	$(CC) pollVectorVoltmeter.o nettools.o -o pollVectorVoltmeter

pollCounters: pollCounters.o nettools.o
	$(CC) pollCounters.o nettools.o -o pollCounters

pollMaser: pollMaser.o nettools.o
	$(CC) pollMaser.o nettools.o -o pollMaser

pollCounters_maser: pollCounters_maser.o nettools.o
	$(CC) pollCounters_maser.o nettools.o -o pollCounters_maser

pollCounters_both: pollCounters_both.o nettools.o
	$(CC) pollCounters_both.o nettools.o -o pollCounters_both

install:
	cp pollVectorVoltmeter $(OUTDIR)/
	cp pollCounters $(OUTDIR)/
	cp pollMaser $(OUTDIR)/
