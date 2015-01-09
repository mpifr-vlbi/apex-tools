
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

./sensors
Some 'lm-sensors' package related things to allow Mark6
temperatures to be read out.

./zbt
Some zero baseline test relate scripts.
