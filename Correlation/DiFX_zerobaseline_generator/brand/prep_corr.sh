
if [ "$1" == "" ]; then
	echo "pre_corr.sh <lo|hi> <percent>"
else
	if [ "$1" == "hi" ]; then
		export band="2048-4096"
	else
		export band="0-2048"
	fi

	# 1 second scans, 13 March
	#scannameA=recordings/ZB_${band}_${2}pc_a.vdif # e.g. recordings/ZB_2048-4096_100pc_a.vdif
	#scannameB=recordings/ZB_${band}_${2}pc_b.vdif # e.g. recordings/ZB_2048-4096_100pc_b.vdif

	# 5 second scands, 16 March (timestamp 2016y006d)
	#scannameA=recordings/ZBCD_5sec_${band}_${2}pc_a.vdif # e.g. recordings/ZBCD_5sec_2048-4096_100pc_a.vdif
	#scannameB=recordings/ZBCD_5sec_${band}_${2}pc_b.vdif # e.g. recordings/ZBCD_5sec_2048-4096_100pc_b.vdif

	# 1 seconds scans, 16 march
	scannameA=recordings/ZBCD_${band}_${2}pc_a.vdif   # e.g. recordings/ZBCD_2048-4096_100pc_a.vdif
	scannameB=recordings/ZBCD_${band}_${2}pc_b.vdif   # e.g. recordings/ZBCD_2048-4096_100pc_b.vdif

	jobname=$1$2
	rm -rf ${jobname}_*.*

	echo "Generating DiFX job files with : ./generate_zerobaseline_vex.sh $scannameA $scannameB $jobname"

	./generate_zerobaseline_vex.sh $scannameA $scannameB $jobname

fi

