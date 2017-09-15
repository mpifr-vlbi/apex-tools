#
# Makefile for dplane
#



#
# PF_RING
#
PFRINGDIR  = /usr/local/lib
LIBPFRING  = ${PFRINGDIR}/libpfring.a


#
# DNA Support
DNA_DEFINE = #-DENABLE_DNA_SUPPORT

#
# PF_RING aware libpcap
#
O_FLAG     = #-O2
EXTRA_LIBS = #-ldag
PCAPDIR    = /usr/local/lib
LIBPCAP    = ${PCAPDIR}/libpcap.a  ${EXTRA_LIBS}

#
# Search directories
#
PFRING_KERNEL=../../kernel
INCLUDE    = -I${PFRING_KERNEL} -I${PFRINGDIR} -I${PCAPDIR}

#
# C compiler and flags
#
#
# CROSS_COMPILE=arm-mv5sft-linux-gnueabi-
#
CC         = ${CROSS_COMPILE}gcc #--platform=native
CFLAGS     = -g ${O_FLAG} -Wall -Wno-parentheses ${INCLUDE} ${DNA_DEFINE} #-O
# LDFLAGS  =

#
# User and System libraries
#
LIBS       = ${LIBPFRING} ${LIBPCAP} -lpthread -lrt

# How to make an object file
%.o: %.c
#	@echo "=*= making object $@ =*="
	${CC} ${CFLAGS} -c $< -o $@

#
# Main targets
#
PFPROGS   = dplane

TARGETS   = ${PFPROGS}

all: dplane

dplane: dplane.o spooler.o phylum.o miscellany.o initialize.o \
    message_handler.o periodic_services.o sequencer.o \
    scat_write.o writer.o ${LIBPFRING} ${LIBPCAP}
	${CC} dplane.o spooler.o phylum.o miscellany.o initialize.o \
    message_handler.o periodic_services.o sequencer.o \
    scat_write.o writer.o ${LIBS} -o $@

install:
	chgrp mark6 dplane
	chmod 4775 dplane
	mv dplane /usr/local/bin

clean:
	@rm -f ${TARGETS} *.o *~

dplane.o initialize.o message_handler.o miscellany.o periodic_services.o \
phylum.o sequencer.o spooler.o scat_write.o writer.o: dplane.h messages.h vdif.h mk5b.h
dplane.o: version.h
