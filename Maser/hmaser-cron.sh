#!/bin/bash

wget -q ftp://iMaser:ndcu@10.0.2.96/Monitoring/MonitInst.dat -O /home/oper/maser/iMaser56-MonitInst.dat  || true
wget -q ftp://iMaser:ndcu@10.0.2.96/Monitoring/MonitMin.dat -O /home/oper/maser/iMaser56-MonitMin.dat  || true
wget -q ftp://iMaser:ndcu@10.0.2.96/Monitoring/MonitMax.dat -O /home/oper/maser/iMaser56-MonitMax.dat  || true

DCMD="SETDATE=`date -u +'%Y/%m/%d'`;"
TCMD="SETTIME=`date -u +'%H:%M:%S'`;"

echo "Sending date-time commands $DCMD $TCMD"
echo -e "$DCMD;\r\n" | nc -u -c maser.apex-telescope.org 14000 || true
echo -e "$TCMD;\r\n" | nc -u -c maser.apex-telescope.org 14000 || true

