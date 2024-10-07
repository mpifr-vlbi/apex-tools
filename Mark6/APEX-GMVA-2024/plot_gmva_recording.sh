#!/bin/bash

if [ "$1" == "" ]; then

	echo
	echo "plot_gmva_recording.sh <name of VDIF file without path>"
	echo
	exit
fi

vdifname=$1

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

cd /mark*_fuse/
mkdir -p 1 2
./unmount.eht
./unmount.gmva

./mount.gmva
rm -f hpol.m5spec
rm -f vpol.m5spec
m5spec -nopol  /mark6-?_fuse/1/$vdifname VDIF_8192-2048-8-2 1280 10000 hpol.m5spec
m5spec -nopol  /mark6-?_fuse/2/$vdifname VDIF_8192-2048-8-2 1280 10000 vpol.m5spec

# generate gnuplot config file
outfile=data.gnu
echo "set term 'wxt'" > $outfile
echo "set multiplot layout 2,1 title 'DBBC3 DDC (8x64GHz): $now'" >> $outfile
echo "set style data line" >> $outfile
echo "set grid x" >> $outfile
echo "set xtics 3024,64" >> $outfile
echo "set xlabel 'frequency [MHz]'" >> $outfile
echo "set ylabel 'amplitude'">> $outfile
echo "unset key" >> $outfile
#echo "set title 'DBBC3 DDC: $now'" >> $outfile
#echo "set term 'png'"  >> $outfile
#echo "set out 'data.png'"  >> $outfile
# DBBC3 channel order BBC1-USB BBC1-LSB BBC2-USB ...
#echo " set logscale y" >> $outfile
echo "set title 'Pol 0'" >> $outfile
echo "plot 'hpol.m5spec' u (-1*\$1+0*64+3024):3, \\" >> $outfile
echo "'hpol.m5spec' u (\$1+0*64+3024):2, \\" >> $outfile
echo "'hpol.m5spec' u (-1*\$1+2*64+3024):5, \\" >> $outfile
echo "'hpol.m5spec' u (\$1+2*64+3024):4, \\" >> $outfile
echo "'hpol.m5spec' u (-1*\$1+4*64+3024):7, \\" >> $outfile
echo "'hpol.m5spec' u (\$1+4*64+3024):6, \\" >> $outfile
echo "'hpol.m5spec' u (-1*\$1+6*64+3024):9, \\" >> $outfile
echo "'hpol.m5spec' u (\$1+6*64+3024):8 " >> $outfile
echo "set title 'Pol 1'" >> $outfile
echo "plot 'vpol.m5spec' u (-1*\$1+0*64+3024):3, \\" >> $outfile
echo "'vpol.m5spec' u (\$1+0*64+3024):2, \\" >> $outfile
echo "'vpol.m5spec' u (-1*\$1+2*64+3024):5, \\" >> $outfile
echo "'vpol.m5spec' u (\$1+2*64+3024):4, \\" >> $outfile
echo "'vpol.m5spec' u (-1*\$1+4*64+3024):7, \\" >> $outfile
echo "'vpol.m5spec' u (\$1+4*64+3024):6, \\" >> $outfile
echo "'vpol.m5spec' u (-1*\$1+6*64+3024):9, \\" >> $outfile
echo "'vpol.m5spec' u (\$1+6*64+3024):8 " >> $outfile
echo "unset multiplot" >> $outfile

gnuplot --persist data.gnu

