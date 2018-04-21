#!/bin/bash

# Apex control system, UDP, project book page 89 "Field System Commands to APECS"
# Sends a single command to the APECS system. Does not display the result though.

AP_CTRL_PORT=22122
AP_CTRL_HOST=10.0.2.170  # observer3.apex-telescope.org in 2013 and 2015
AP_LOG=/usr2/log/apecs_cmd.log

TIMEOUT_SECS=2
echo "@`date -u` sent $1" >> $AP_LOG
#echo -e "$1 \n" | netcat -u -w $TIMEOUT_SECS -q $TIMEOUT_SECS $AP_CTRL_HOST $AP_CTRL_PORT >> $AP_LOG
echo -e "\n" >> $AP_LOG
