#!/bin/bash

# Apex control system, UDP, project book page 89 "Field System Commands to APECS"

AP_CTRL_PORT=22122
AP_CTRL_HOST=observer3.apex-telescope.org
AP_LOG=/usr2/log/apexTsys.log

# MANUAL TSYS FOR G1MA01 -- FIRST PART OF SESSION

TIMEOUT_SECS=1
# >1 second needed to delay FieldSystem .snp processing to ensure our UDP packet arrives before 
# the source= UDP packet from antcn.c, so that Tsys is executed while still on source (APEX control 
# has a command queue, filled up from UDP (any network source), and this queue gets executed serially)

# invoke the measurement first

tsyscmd1="execfile('vlbi_calibrate.apecs')"
echo $tsyscmd1 | netcat -u -w $TIMEOUT_SECS -q $TIMEOUT_SECS $AP_CTRL_HOST $AP_CTRL_PORT >> $AP_LOG
echo -e "\n" >> $AP_LOG

# need to wait for a magic amount of time to ensure that APECS has background-processed the data of the completed calibrate() command
sleep 80

# then queue up the "get results" command
tsyscmd1="execfile('vlbi_calibrateget.apecs')"
echo $tsyscmd1 | netcat -u -w $TIMEOUT_SECS -q $TIMEOUT_SECS $AP_CTRL_HOST $AP_CTRL_PORT >> $AP_LOG
echo -e "\n" >> $AP_LOG
