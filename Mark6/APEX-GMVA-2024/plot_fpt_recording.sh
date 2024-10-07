#!/bin/bash

if [ "$1" == "" ]; then

	echo
	echo "Plots Mark6 recordings of DBBC3 OCT_D 1-ch x 1024 MHz mode"
	echo
	echo "Usage: plot_fpt_recording.sh <name of VDIF file without path>"
	echo
	exit
fi

vdifname=$1

cd /mark*_fuse/
mkdir -p 1 2
./unmount.eht
./unmount.gmva  # todo: make a  [un]mount.fpt script?

./mount.gmva
rm -f hpol.m5spec
rm -f vpol.m5spec

if ! [ -f /mark6-?_fuse/1/$vdifname ]; then
	echo
	echo "Error: specified file '$vdifname' not found under /mark6-?_fuse/1/"
	echo
	exit
fi
if ! [ -f /mark6-?_fuse/2/$vdifname ]; then
	echo
	echo "Error: specified file '$vdifname' not found under /mark6-?_fuse/2/"
	echo
	exit
fi

m5spec -nopol  /mark6-?_fuse/1/$vdifname VDIF_8192-4096-1-2 20480 1000 hpol.m5spec
m5spec -nopol  /mark6-?_fuse/2/$vdifname VDIF_8192-4096-1-2 20480 1000 vpol.m5spec

# generate gnuplot config file
outfile=data.gnu
echo "set term 'x11'" > $outfile
echo "set multiplot layout 2,1 title 'DBBC3 OCT_D 1 GHz Mode (1 x 1024 MHz): $now'" >> $outfile
echo "set style data line" >> $outfile
echo "set grid x" >> $outfile
echo "set xlabel 'Frequency [MHz]'" >> $outfile
echo "set ylabel 'Amplitude'">> $outfile
echo "set xrange [0:1024]" >> $outfile
echo "set logscale y" >> $outfile
echo "unset key" >> $outfile
echo "set title 'Pol 0'" >> $outfile
echo "plot 'hpol.m5spec' u 1:2" >> $outfile
echo "set title 'Pol 1'" >> $outfile
echo "plot 'vpol.m5spec' u 1:2" >> $outfile
echo "unset multiplot" >> $outfile

gnuplot --persist data.gnu

