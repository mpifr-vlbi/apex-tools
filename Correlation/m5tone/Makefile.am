
bin_PROGRAMS = m5tone2021

m5tone2021_SOURCES = m5tone_eht2021.c

m5tone2021_CFLAGS = -Wall -g -O3 -ffast-math -fno-math-errno -m64 -march=native -mtune=native $(shell pkg-config --cflags mark5access)
m5tone2021_LDFLAGS = $(shell pkg-config --libs mark5access) -lm -lfftw3f -lgfortran -lX11 -lpng -lmark5access
