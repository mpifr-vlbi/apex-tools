INCLUDES = -I$(includedir)
LDADD =    -lmark5access -lm
bin_PROGRAMS = m5tone2021

m5tone2021_SOURCES = \
        m5tone_eht2021.c

m5tone2021_CFLAGS =  -I/opt/pgplot -L$(libdir) -g -O3 -ffast-math -Wall -fno-math-errno -m64 -march=native -mtune=native 
m5tone2021_LDADD =  $(LDADD)  -lfftw3f -lgfortran -lX11 -lpng /opt/pgplot/libcpgplot.a /opt/pgplot/libpgplot.a
