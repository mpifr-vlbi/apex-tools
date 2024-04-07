#!/bin/bash
#
# Restart the EHTVLBImonitor client.py script
#
# Seems needed, since sometimes the connection to APECS remains
# intact yet all queries for monitoring data begin to fail.
#
# This script is intended to be paired with an hourly restart via
# $ crontab -e
#   # Hourly restart of EHTVLBImonitor client.py
#   0 * * * *  /homes/t-0113.f-9996a-2024/EHTVLBImonitor/reanimate-EHTVLBImonitor.sh >/dev/null 2>&1
#

PIDFILE=/tmp/vlbimonclient.pid

if [ -f $PIDFILE ]; then
	oldpid=`cat $PIDFILE`
	if [ ! -z "$oldpid" ]; then
		echo "Killing old python instance running client.py under PID $oldpid"
		kill -9 $oldpid
		rm -f $PIDFILE
	fi
fi

# su -s /bin/bash -c "nohup /alma/ACS-2021DEC/pyenv/shims/python2 client.py &" `whoami`  # downside: asks for passwd
nohup /alma/ACS-2021DEC/pyenv/shims/python2 client.py &
disown
# NB: client.py has to write $PIDFILE on its own
# TODO: background python2 and store bash $$ pid into $PIDFILE


