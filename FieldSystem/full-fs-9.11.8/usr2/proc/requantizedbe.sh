#!/bin/bash

echo "requantizedbe.sh obsolete for 2015"
exit


# for 2012:

edrs
fusermount -u /mnt/diskpack

cd /recordings/fs/

export DBE_IP=10.0.2.101    # IP of DBE telnet
export DBE_REFGAIN=4500     # determined earlier, manually; approximate gain for 1st iteration

/home/oper/APEX/DBE/dbe/dbers232 -u $DBE_IP 10001 -g $DBE_REFGAIN
sleep 1

rm -f capture.m5b

mk5netdump 5008 40 capture.m5b $((2*1024*1024))
m5bstate capture.m5b Mark5B-512-16-2 1000 | tail -16 > capture.m5bstate_before

/home/oper/APEX/DBE/dbe/dbers232 -u $DBE_IP 10001 -sgfile0 capture.m5bstate_before

mk5netdump 5008 40 capture.m5b $((2*1024*1024))
m5bstate capture.m5b Mark5B-512-16-2 1000 | tail -16 > capture.m5bstate_after

echo "--- Bit state statistics with 1st iteration gain of $DBE_REFGAIN ---"
cat capture.m5bstate_before

echo "--- Bit state statistics with per-channel gains applied ---"
cat capture.m5bstate_after

## -- manually: drs.py
