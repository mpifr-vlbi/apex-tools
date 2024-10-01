#!/bin/bash
#
# H-maser logfile and timekeeping script to execute periodically.
#
# 1) downloads monitoring data logs from the H-maser
# 2) uploads the logs (1) to T4Science/Safran FTP for their own product health monitoring
# 3) refreshes the UTC date and time of the embedded PC in the H-maser for
#
# This script is ideally executed hourly via CRON, e.g.,
# $ crontab -e
#   0 * * * *       /home/oper/bin/hmaser-cron.sh 2>&1 > /dev/null
#

LOGDIR=/home/oper/maser/

function upload_to_t4s_dummy()
{
echo "Uploading log to T4 Science... (dummy func)"
}

function upload_to_t4s()
{
echo "Uploading log to T4 Science..."
pushd $LOGDIR
ftp -n -v www.t4science.ch << EOT
ascii
user chqw_mpi iMaser56
prompt
passive
put iMaser56-MonitInst.dat
put iMaser56-MonitMax.dat
put iMaser56-MonitMin.dat
EOT
popd
}

## Grab newest logs from the H-maser
wget -q ftp://iMaser:ndcu@10.0.2.96/Monitoring/MonitInst.dat -O $LOGDIR/iMaser56-MonitInst.dat  || true
wget -q ftp://iMaser:ndcu@10.0.2.96/Monitoring/MonitMin.dat -O $LOGDIR/iMaser56-MonitMin.dat  || true
wget -q ftp://iMaser:ndcu@10.0.2.96/Monitoring/MonitMax.dat -O $LOGDIR/iMaser56-MonitMax.dat  || true

## Upload to T4Sscience / Safran
pushd $LOGDIR
if [ -s iMaser56-MonitInst.dat ]; then
        upload_to_t4s
else
        echo "Error: File iMaser56-MonitInst.dat appears to be empty! Not uploading by FTP to T4S."
fi
popd

## Force update of H-maser datetime
DCMD="SETDATE=`date -u +'%Y/%m/%d'`;"
TCMD="SETTIME=`date -u +'%H:%M:%S'`;"
echo "Sending date-time commands $DCMD $TCMD"
echo -e "$DCMD\r" | nc -c -u -w 1 maser.apex-telescope.org 14000
echo -e "$TCMD\r" | nc -c -u -w 1 maser.apex-telescope.org 14000
