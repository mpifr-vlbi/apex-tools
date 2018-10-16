#!/bin/bash

# Apex control system, UDP, project book page 89 "Field System Commands to APECS"

AP_CTRL_PORT=22122
AP_CTRL_HOST=observer3.apex-telescope.org
AP_LOG=/usr2/log/apexVLBI-2015.log

TIMEOUT_SECS=1

cmd1="execfile('vlbi_commands.py')"
cmd2="vlbi_init()"

echo $cmd1 | netcat -u -w $TIMEOUT_SECS -q $TIMEOUT_SECS $AP_CTRL_HOST $AP_CTRL_PORT >> $AP_LOG
echo -e "\n" >> $AP_LOG

echo $cmd2 | netcat -u -w $TIMEOUT_SECS -q $TIMEOUT_SECS $AP_CTRL_HOST $AP_CTRL_PORT >> $AP_LOG
echo -e "\n" >> $AP_LOG

