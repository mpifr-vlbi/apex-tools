if [ "$2" == "" ]; then

    echo "Usage: remote_capture.sh <recorder_hostname> <recorder_hostname>"
    echo ""
    echo "Captures VDIF data of IF0 from the first and second host"

else

    rm -f recorder*-if?.vdif

    ssh $1 vdifsnapshotUDP --offset=8 256 4000 /home/oper/zbt/$1-if0.vdif &
    ssh $2 vdifsnapshotUDP --offset=8 256 4000 /home/oper/zbt/$2-if0.vdif &
    sleep 4
    scp $1:/home/oper/zbt/$1-if0.vdif .
    scp $2:/home/oper/zbt/$2-if0.vdif .

    echo "You can now do:"
    echo "python /usr/local/src/r2dbe-outdatedRepo/software/vdif_corr.py --nfft 1024 -n 1024 -s 4096 $1-if0.vdif $2-if0.vdif"
fi
