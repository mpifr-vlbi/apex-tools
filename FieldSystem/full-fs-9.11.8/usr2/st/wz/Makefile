CFLAGS= -m32
FFLAGS= -m32
# /usr2/st/stqkr/Makefile
#
# Makefile for RTW FS stqkr (station quikr)
# last mod. 04_06_2001, zei pttest added.
# 18.06.2008 dbbc testversion added, zei.
FFLAGS= -m32  -c
LIBES =  ../../fs/clib/clib.a ../../fs/rtelb/rtelb.a ../../st/stlib/stlib.a\
         ../../fs/port/port.a ../../fs/poclb/poclb.a\
         /usr/lib/libm.a
#
OBJECTS = \
stqkr.o wx.o wxz.o point.o mpoint.o cable.o counter2.o onso.o relheight.o\
ptlog.o \
dbbc/dbbc.o dbbc/dbbcnn.o dbbc/dbbcif.o dbbc/dbbcifn.o\
dbbc/dbbcform.o dbbc/dbbcmon.o dbbc/dbbc_cal_if.o dbbc/dbbc_cal_ch.o\
dbbc/simple_socket.o dbbc/dbbc_com.o dbbc/logfile.o dbbc/resetall.o\
dbbc/clock.o dbbc/dbbcstat.o dbbc/reg.o dbbc/find_control.o dbbc/dbc_com.o\
../antlib/antlib.o
#
../../st/bin/stqkr: $(OBJECTS)
	$(FC) -Wl,-m32 -o ../../st/bin/stqkr $(OBJECTS) $(LIBES)
	strip ../../st/bin/stqkr
#
stqkr:
	rm -f ../../st/bin/stqkr
	make
#
