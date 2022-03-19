#!/bin/bash
# Helper script to mount both the recorder module(s) (fuseMk6)
# and mount that remote mount onto eht-cc (sshfs)

usage() { echo "Usage: $0 recorder slot" 1>&2; exit 1; }

if [ $# -lt 2 ]; then
    usage
fi

host=$1
slots=$2

sshfsopts="-o cache=no -o cipher=arcfour"

echo "Trying to mount slot(s) $slots on $host"

remotemount=/home/oper/fuse/$slots
localmount=/home/oper/transfer/${host}_${slots}
echo $localmount
fusermount3 -u $localmount

ssh oper@$host 'fusermount -u '''$remotemount''''
ssh -t oper@$host 'mkdir -p '''$remotemount''''
ssh oper@$host 'chmod a+rwx '''$remotemount''''
ssh oper@$host '/usr/local/difx/bin/fuseMk6 -r "/mnt/disks/['''$slots''']/*/data" '''$remotemount''''

mkdir -p $localmount
sshfs $host:$remotemount  $localmount $sshfsopts
echo "Mounted slots $slots of $host locally at $localmount"

