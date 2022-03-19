#!/bin/bash
# Helper script to unmount both the recorder module(s) (fuseMk6)
# and unmount that remote mount from eht-cc (sshfs)

usage() { echo "Usage: $0 recorder slot" 1>&2; exit 1; }

if [ $# -lt 2 ]; then
    usage
fi

host=$1
slots=$2

sshfsopts="-o cache=no -o cipher=arcfour"

echo "Trying to unmount slot(s) $slots on $host"

remotemount=/home/oper/fuse/$slots
localmount=/home/oper/transfer/${host}_${slots}
echo $localmount
fusermount3 -u $localmount

ssh oper@$host 'fusermount -u '''$remotemount''''

