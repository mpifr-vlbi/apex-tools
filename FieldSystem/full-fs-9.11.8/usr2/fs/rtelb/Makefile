CFLAGS= -m32
FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
#
OBJECTS=\
rte_alarm.o rte_sleep.o rte_time.o rte_prior.o rte_lock.o\
rte2secs.o rte_rawt.o rte_sett.o rte_fixt.o rte_secs.o rte_cmpt.o secs2rte.o\
rte_ticks.o secs_time.o rte_times.o rte_check.o
#
CFLAGS = -m32  
rtelb.a: $(OBJECTS)
	touch rtelb.a
	rm rtelb.a
	ar -qc rtelb.a $(OBJECTS)
	ar s rtelb.a
