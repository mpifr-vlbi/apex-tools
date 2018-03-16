
if [ "$1" == "" ]; then

	echo "Usage: generate_zerobaseline_vex.sh <vdiffile1> <vdiffile2> <jobname>"
	echo
	echo "Produces:"
	echo "   zerobaseline_file1.lst pointing to VDIF file 1"
	echo "   zerobaseline_file2.lst pointing to VDIF file 2"
	echo "   zerobaseline.vex"
	echo "   jobname.v2d  referring to zerobaseline.vex and zerobaseline_file[1-2].lst"
	echo
	#echo " TODO: sed to modify jobname.v2d so it points to jobname.vex and jobname_file[1-2].lst "

else

	echo "Running vsum -s $1..."
	export times=`vsum -s $1`
	echo $times > zerobaseline_file1.lst
	vsum -s  $2 > zerobaseline_file2.lst

	export mjdstart=`echo $times | cut -d " " -f 2`
	export vexfmt=`vdif_time -q Vex -p 0 $mjdstart`

	echo "Got MJD at VDIF start of $mjdstart, converted to VEX format $vexfmt"

	sed "s/2017y000d00h00m00s/$vexfmt/g" generate_zerobaseline_vex.template > zerobaseline.vex
	cp -a zerobaseline.vex ${3}.vex
	cp -a zerobaseline.v2d ${3}.v2d
	cp -a zerobaseline_file1.lst ${3}_file1.lst
	cp -a zerobaseline_file2.lst ${3}_file2.lst
	sed -i "s/vex = zerobaseline.vex/vex = ${3}.vex/g" ${3}.v2d
	sed -i "s/filelist = zerobaseline_file1.lst/filelist = ${3}_file1.lst/g" ${3}.v2d
	sed -i "s/filelist = zerobaseline_file2.lst/filelist = ${3}_file2.lst/g" ${3}.v2d

	echo "Generated ${3}.v2d "
	vex2difx $3.v2d

fi

