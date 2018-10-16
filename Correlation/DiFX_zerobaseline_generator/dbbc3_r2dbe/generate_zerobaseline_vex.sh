
# Modified for 3-station
# DBBC3 IF-C  x   DBBC3 IF-D   x   R2DBE if0

if [ "$1" == "" ]; then

	echo "Usage: generate_zerobaseline_vex.sh <dbbc3_vdiffile1> <dbbc3_vdiffile2> <r2dbe_vdiffile3> <jobname>"
	echo
	echo "Produces:"
	echo "   zerobaseline_file1.lst pointing to VDIF file 1; DBBC3 IF-C"
	echo "   zerobaseline_file2.lst pointing to VDIF file 2; DBBC3 IF-D"
	echo "   zerobaseline_file3.lst pointing to VDIF file 3; R2DBE if0"
	echo "   jobname.vex"
	echo "   jobname.v2d  referring to jobmame.vex and zerobaseline_file[1-3].lst"
	echo

else

	echo "Running vsum -s $1..."
	export times=`vsum -s $1`
	echo $times > zerobaseline_file1.lst
	vsum -s  $2 > zerobaseline_file2.lst
	vsum -s  $3 > zerobaseline_file3.lst

	export mjdstart=`echo $times | cut -d " " -f 2`
	export vexfmt=`vdif_time -q Vex -p 0 $mjdstart`

	echo "Got MJD at VDIF start of $mjdstart, converted to VEX format $vexfmt"

	sed "s/2017y000d00h00m00s/$vexfmt/g" generate_zerobaseline_vex.template > ${4}.vex
	cp -a generate_zerobaseline_vex.v2d_template ${4}.v2d
	cp -a zerobaseline_file1.lst ${4}_file1.lst
	cp -a zerobaseline_file2.lst ${4}_file2.lst
	cp -a zerobaseline_file3.lst ${4}_file3.lst
	sed -i "s/vex = zerobaseline.vex/vex = ${4}.vex/g" ${4}.v2d
	sed -i "s/filelist = zerobaseline_file1.lst/filelist = ${4}_file1.lst/g" ${4}.v2d
	sed -i "s/filelist = zerobaseline_file2.lst/filelist = ${4}_file2.lst/g" ${4}.v2d
	sed -i "s/filelist = zerobaseline_file3.lst/filelist = ${4}_file3.lst/g" ${4}.v2d

	echo "Generated ${4}.v2d "
	vex2difx $4.v2d

fi

