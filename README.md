
The files in this repository are related to APEX mm-VLBI 
configuration and station setup. 

-------------------------------------------------------------

To download the files:
$ git clone https://bitbucket.org/jwagner313/apex-tools

To upload changes:
$ cd apex-tools ; git commit -m "message" ; git push

To update to the most recent version:
$ cd apex-tools ; git pull

-------------------------------------------------------------

The subdirectories are

./APECS 
Some VLBI related files for the APEX telescope control system

./DBBC2
Some DBBC2 related files for APEX telescope VLBI

   consoleFILA10G.py
   Serial console with automatic command file exec,
   based on https://bitbucket.org/jwagner313/fila10gtools,
   with minor modification for APEX. Settings file included.

./GPIB-tools
Tools to read VLBI related GPIB counter data, with some
improvements over their earlier year 2013 version.

   pollCounters.c
   Queries GPIB counters, writes data to pollCounters.out   

   pollMaser.c
   Queries the H-maser data

   pollCounters_fit_rate.m
   Octave/Matlab script to fit linear trend through the data,
   e.g., estimate the H-maser drift rate against GPS

./R2DBE
Settings and new tools related to the Roach2 Digital Backend,
some of them later to be found at https://github.com/sma-wideband

   APEX_config.py 
   Based on VDIF_test.py from the SMA wideband correlator git repository.
   Configures the R2DBE for use at APEX (clock phase cal, time sync, VDIF mode (currently
   VDIF epoch is not determined automatically from current UT time!), network/IPs,
   auto-adjustment of 2-bit conversion thresholds, ...)

   m5specR2DBE.c
   Spectrometer identical to m5spec from mark5access/DiFX but with minor bugfix
   to handle 2048 MHz wide bands.

   quickspecR2DBE.py
   New R2DBE quick test spectrometer. Uses the 8-bit sample snapshots from the ADC directly,
   rather than capturing any 10 GbE data.

   requantizeR2DBE.py
   Automatically adjust 2-bit conversion thresholds.
   Needed when IF power level changes such as after the telescope switches to new target.

./sensors
Some 'lm-sensors' package related things to allow Mark6
temperatures to be read out.
  
   nct6775
   Official nct6775 sensor driver (on P9X79-E WS motherboard of Mark6), with minor
   modifications, to replace the very outdated driver of the Debian distro running 
   on the Mark6's.
   Not sure if temperature readings other than from AUXTIN (board/chassis/chipset)
   are reliable. 

./zbt
Some zero baseline test relate scripts.
