#!/bin/bash
#
#  Usage:   mk6rec.sh [<mark 6 ip address>]
#
#  Tells Mark 6 to start recording 5 seconds from now
#  and record 30 data seconds.
#

MK6_IP=$1
if [ "$1" == "" ]; then
    MK6_IP=127.0.0.1   # default to local computer
fi

# Get UT time 5 seconds in the future, in VEX format (like 2015y017d06h00m00s)
vextime=`date -u --date='5 seconds' +%Yy%jd%Hh%Mm%Ss`  
scanname=`date -u --date='5 seconds' +%j-%H%M`

# Use netcat (nc) to send Mark 6 the command to record 30 seconds
cmd="record=$vextime:30:30:$scanname:test:KT;\n"
echo "Sending $cmd to Mark6 at $MK6_IP"
echo -e $cmd | nc -w 1 $MK6_IP 14242

