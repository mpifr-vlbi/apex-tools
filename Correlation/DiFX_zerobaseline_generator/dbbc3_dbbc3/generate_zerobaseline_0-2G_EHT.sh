#!/bin/bash

if [ "$1" == "" ] || [ "$2" == "" ]; then

	echo "Usage: generate_zerobaseline_0-2G_EHT.sh <scan name without path.vdif> <jobname>"
	echo
	echo "For simultaneous recordings made with e.g. backendctl mark6 all run test-recording 5 5."
	echo "This script produces:"
	echo "   jobname_rec1.{12,34}.lst pointing to VDIF file slot 12,34 on recorder1"
	echo "   jobname_rec2.{12,34}.lst pointing to VDIF file slot 12,34 on recorder2"
	echo "   jobname.vex"
	echo "   jobname.v2d  referring to jobmame.vex and jobname_rec{1,2}.{12,34}.lst"
	echo

else

	echo "Running vsum -s /mark6-?_fuse/12/$1..."
	export times=`vsum -s /mark6-?_fuse/12/$1`
	echo $times > ${2}_rec1.12.lst
	echo $times > ${2}_rec1.34.lst
	echo $times > ${2}_rec2.12.lst
	echo $times > ${2}_rec2.34.lst
	sed -i "s/mark6-1/mark6-2/g" ${2}_rec2.12.lst
	sed -i "s/mark6-1/mark6-2/g" ${2}_rec2.34.lst

	export mjdstart=`echo $times | cut -d " " -f 2`
	export vexfmt=`vdif_time -q Vex -p 0 $mjdstart`

	echo "Got MJD at VDIF start of $mjdstart, converted to VEX format $vexfmt"

	sed "s/2017y000d00h00m00s/$vexfmt/g" generate_zerobaseline_0-2G_EHT.template > ${2}.vex
	cp -a generate_zerobaseline_0-2G_EHT.v2d_template ${2}.v2d
	sed -i "s/vex = zerobaseline.vex/vex = ${2}.vex/g" ${2}.v2d
	sed -i "s/filelist = zerobaseline_rec1.12.lst/filelist = ${2}_rec1.12.lst/g" ${2}.v2d
	sed -i "s/filelist = zerobaseline_rec1.34.lst/filelist = ${2}_rec1.34.lst/g" ${2}.v2d
	sed -i "s/filelist = zerobaseline_rec2.12.lst/filelist = ${2}_rec2.12.lst/g" ${2}.v2d
	sed -i "s/filelist = zerobaseline_rec2.34.lst/filelist = ${2}_rec2.34.lst/g" ${2}.v2d

	# EOP, actual: getEOP.py --local $vexfmt  # expects env vars DIFX_EOPS=usno_finals.erp  DIFX_UT1LS=ut1ls.dat
	# EOP, filler since not critical for zero baseline:
	mjd0=${mjdstart%%.*}  # float to int
	mjd0=$((mjd0 - 2))
	echo "" >> ${2}.v2d
	echo "## dummy EOPs" >> ${2}.v2d
	echo "EOP $((mjd0+0)) { xPole=0.049570 yPole=0.423290 tai_utc=37 ut1_utc=0.116485 }" >> ${2}.v2d
	echo "EOP $((mjd0+1)) { xPole=0.049570 yPole=0.423290 tai_utc=37 ut1_utc=0.116485 }" >> ${2}.v2d
	echo "EOP $((mjd0+2)) { xPole=0.049570 yPole=0.423290 tai_utc=37 ut1_utc=0.116485 }" >> ${2}.v2d
	echo "EOP $((mjd0+3)) { xPole=0.049570 yPole=0.423290 tai_utc=37 ut1_utc=0.116485 }" >> ${2}.v2d
	echo "EOP $((mjd0+4)) { xPole=0.049570 yPole=0.423290 tai_utc=37 ut1_utc=0.116485 }" >> ${2}.v2d

	echo "Generated ${2}.v2d "
	vex2difx ${2}.v2d

fi
