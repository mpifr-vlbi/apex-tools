#!/bin/bash

# Apex control system, UDP, project book page 89 "Field System Commands to APECS"

AP_CTRL_PORT=22122
AP_CTRL_HOST=observer3.apex-telescope.org

TIMEOUT_SECS=2 
# >1 second needed to delay FieldSystem .snp processing to ensure our UDP packet arrives before 
# the source= UDP packet from antcn.c, so that Tsys is executed while still on source (APEX control 
# has a command queue, filled up from UDP (any network source), and this queue gets executed serially)

tsyscmd1="execfile('vlbi_calibrateget.apecs')"

echo $tsyscmd1 | netcat -u $AP_CTRL_HOST $AP_CTRL_PORT -w $TIMEOUT_SECS -q $TIMEOUT_SECS >> /usr2/log/apexTsys.log
echo -e "\n" >> /usr2/log/apexTsys.log
