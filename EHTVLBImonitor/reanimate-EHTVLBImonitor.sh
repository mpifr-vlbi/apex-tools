#!/bin/bash

PIDFILE=/tmp/vlbimonclient.pid

if [ -f $PIDFILE ]; then
	oldpid=`cat $PIDFILE`
	if [ ! -z "$oldpid" ]; then
		echo "Killing old python instance running client.py under PID $oldpid"
		kill -9 $oldpid
		rm -f $PIDFILE
	fi
fi

python2 client.py
# NB: client.py has to write $PIDFILE on its own
# TODO: background python2 and store bash $$ pid into $PIDFILE


