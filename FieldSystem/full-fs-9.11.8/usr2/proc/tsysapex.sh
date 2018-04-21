#!/bin/bash

# Apex control system, UDP, project book page 89 "Field System Commands to APECS"

AP_CTRL_PORT=22122
AP_CTRL_HOST=observer3.apex-telescope.org
AP_LOG=/usr2/log/apexTsys.log


# Timeout / delay:

# >1.0 seconds needed to delay FieldSystem .snp processing to ensure our UDP packet arrives before 
# the source= UDP packet from antcn.c, so that Tsys is executed while still on source (APEX control 
# has a command queue, filled up from UDP (any network source), and this queue gets executed serially)
TIMEOUT_SECS=1


# Invoke Tsys (Sky-Hot-Cold) measurement via APECS Remote exec

tsyscmd1="execfile('vlbi_calibrate.apecs')"
echo $tsyscmd1 | netcat -u -w $TIMEOUT_SECS -q $TIMEOUT_SECS $AP_CTRL_HOST $AP_CTRL_PORT >> $AP_LOG
echo -e "\n" >> $AP_LOG

# The results come from APECS via Event Listener that calls 'inject_snap' on the master fieldsystem machine.
# The event listener must be running on e.g. obsever3. It is called ./calibrator_events/processCalEvents
# on the observer3 machine. Overall the process of triggering and getting Tsys is quite convoluted...
