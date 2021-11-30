
CFLAGS = -Wall -g -O3 -ffast-math -fno-math-errno -m64 -march=native -mtune=native $(shell pkg-config --cflags mark5access)

LDFLAGS = $(shell pkg-config --libs mark5access) -lm -lfftw3f -lgfortran -lX11 -lpng -lmark5access

bin_PROGRAMS = \
	m5tone_Ax \
	m5tone_Ax_2018_vdi \
	m5tone_Ax_2018 \
	m5tone_Kt

m5tone_Ax_SOURCES = m5tone_eht.c
m5tone_Ax_CFLAGS = -DSTATION_APEX

m5tone_Ax_2018_vdi_SOURCES = m5tone_eht.c
m5tone_Ax_2018_vdi_CFLAGS = -DSTATION_APEX_2018_VDI  # VDI synth for 1st LO with LO offset

m5tone_Ax_2018_SOURCES = m5tone_eht.c
m5tone_Ax_2018_CFLAGS = -DSTATION_APEX_2018_RS  # without LO offset

m5tone_Kt_SOURCES = m5tone_eht.c
m5tone_Kt_CFLAGS = -DSTATION_KITTPEAK
