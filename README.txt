
The files in this repository are related to APEX mm-VLBI 
configuration and station setup.

The subdirectories are

./APECS 
Some VLBI related files for the APEX telescope control system

./DBBC2
Some DBBC2 related files for APEX telescope VLBI

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
Tools and settings related to the Roach2 Digital Backend

./sensors
Some 'lm-sensors' package related things to allow Mark6
temperatures to be read out.

./zbt
Some zero baseline test relate scripts.
