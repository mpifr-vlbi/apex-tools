####################################################################################################################################################

AM_CFLAGS = -pthread -Wall -g -O3 -ffast-math -fno-math-errno -m64 -march=native -mtune=native $(shell pkg-config --cflags mark5access,fftw3f)

AM_LDFLAGS = $(shell pkg-config --libs mark5access,fftw3f) -pthread

####################################################################################################################################################

bin_PROGRAMS = \
	m5tone_Ax \
	m5tone_Ax_2018_vdi \
	m5tone_Ax_2022_vdi \
	m5tone_Ax_2023_vdi \
	m5tone_Ax_2024_vdi_260G \
	m5tone_Ax_2018 \
	m5tone_Kt \
	m5tone_threaded_Ax \
	m5tone_threaded_Ax_2018_vdi \
	m5tone_threaded_Ax_2022_vdi \
	m5tone_threaded_Ax_2023_vdi \
	m5tone_threaded_Ax_2024_vdi_260G \
	m5tone_threaded_Ax_2018 \
	m5tone_threaded_Kt \
	m5tone_mpi_Ax_2022_vdi \
	m5tone_mpi_Ax_2023_vdi \
	m5tone_mpi_Ax_2024_vdi_260G

####################################################################################################################################################

m5tone_Ax_SOURCES = m5tone_eht.c
m5tone_Ax_CFLAGS = $(AM_CFLAGS) -DSTATION_APEX

m5tone_threaded_Ax_SOURCES = m5tone_threaded_eht.c
m5tone_threaded_Ax_CFLAGS = $(AM_CFLAGS) -DSTATION_APEX

####################################################################################################################################################

# 2018 Rohde-Schwarz synth for 1st LO without any LO offset for tone in band 3

m5tone_Ax_2018_SOURCES = m5tone_eht.c
m5tone_Ax_2018_CFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2018_RS

m5tone_threaded_Ax_2018_SOURCES = m5tone_threaded_eht.c
m5tone_threaded_Ax_2018_CFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2018_RS

# 2018 VDI synth for 1st LO with -15.022 LO offset for tone in band 3

m5tone_Ax_2018_vdi_SOURCES = m5tone_eht.c
m5tone_Ax_2018_vdi_CFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2018_VDI

m5tone_threaded_Ax_2018_vdi_SOURCES = m5tone_threaded_eht.c
m5tone_threaded_Ax_2018_vdi_CFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2018_VDI

####################################################################################################################################################

# 2022 VDI synth for 230 GHz 1st LO with +15.022 Hz LO offset for tone in band 1

m5tone_Ax_2022_vdi_SOURCES = m5tone_eht.c
m5tone_Ax_2022_vdi_CFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2022_VDI

m5tone_threaded_Ax_2022_vdi_SOURCES = m5tone_threaded_eht.c
m5tone_threaded_Ax_2022_vdi_CFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2022_VDI

m5tone_mpi_Ax_2022_vdi_SOURCES = m5tone_mpi_eht.cpp
m5tone_mpi_Ax_2022_vdi_CXXFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2022_VDI

####################################################################################################################################################

# 2023 & 2024 VDI synth for 230 GHz 1st LO with -15.022 Hz LO offset for tone in band 4, 345 GHz no tone

m5tone_Ax_2023_vdi_SOURCES = m5tone_eht.c
m5tone_Ax_2023_vdi_CFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2023_VDI

m5tone_threaded_Ax_2023_vdi_SOURCES = m5tone_threaded_eht.c
m5tone_threaded_Ax_2023_vdi_CFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2023_VDI

m5tone_mpi_Ax_2023_vdi_SOURCES = m5tone_mpi_eht.cpp
m5tone_mpi_Ax_2023_vdi_CXXFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2023_VDI

####################################################################################################################################################

# 2024 VDI synth for 260 GHz with larger -56.86 Hz LO offset for tone in band 4

m5tone_Ax_2024_vdi_260G_SOURCES = m5tone_eht.c
m5tone_Ax_2024_vdi_260G_CFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2024_VDI_260G

m5tone_threaded_Ax_2024_vdi_260G_SOURCES = m5tone_threaded_eht.c
m5tone_threaded_Ax_2024_vdi_260G_CFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2024_VDI_260G

m5tone_mpi_Ax_2024_vdi_260G_SOURCES = m5tone_mpi_eht.cpp
m5tone_mpi_Ax_2024_vdi_260G_CXXFLAGS = $(AM_CFLAGS) -DSTATION_APEX_2024_VDI_260G

####################################################################################################################################################

m5tone_Kt_SOURCES = m5tone_eht.c
m5tone_Kt_CFLAGS = $(AM_CFLAGS) -DSTATION_KITTPEAK

m5tone_threaded_Kt_SOURCES = m5tone_threaded_eht.c
m5tone_threaded_Kt_CFLAGS = $(AM_CFLAGS) -DSTATION_KITTPEAK

####################################################################################################################################################
