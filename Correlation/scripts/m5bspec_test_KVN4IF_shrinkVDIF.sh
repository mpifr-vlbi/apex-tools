
# vsi1-2-3-4 data captured when chan_perm is set to defaults
# This produces VDIF data where payload does not follow standard
# sample time x frequency layout. Using 'shrinkVDIF' to extract
# each band from such a VDIF file to produce more standard VDIF
# payload sample layout.

rm -f m5spec_test.vdif vsi1.vdif vsi2.vdif vsi3.vdif vsi4.vdif vsi1.m5spec vsi2.m5spec vsi3.m5spec vsi4.m5spec

vdifsnapshotUDP --cpu=2 --offset=8 128 46227 m5spec_test.vdif

# Frame size 8192+32 reduces to 2048+32 by taking only one 32-bit VSI interface
# Data rate of 8192 Mbps (4 VSI) reduces to single-VSI rate of 2048 Mbps
shrinkVDIF -x 4 m5spec_test.vdif vsi1.vdif
shrinkVDIF -x 3 m5spec_test.vdif vsi2.vdif
shrinkVDIF -x 2 m5spec_test.vdif vsi3.vdif
shrinkVDIF -x 1 m5spec_test.vdif vsi4.vdif

m5spec -nopol vsi1.vdif VDIF_2048-2048-1-2 8192 2000 vsi1.m5spec
m5spec -nopol vsi2.vdif VDIF_2048-2048-1-2 8192 2000 vsi2.m5spec
m5spec -nopol vsi3.vdif VDIF_2048-2048-1-2 8192 2000 vsi3.m5spec
m5spec -nopol vsi4.vdif VDIF_2048-2048-1-2 8192 2000 vsi4.m5spec

echo "plot \"vsi1.m5spec\" using (\$1+0):2 with lines title \"VSI1\", \\" > cmd
echo "     \"vsi2.m5spec\" using (\$1+512):2 with lines title \"VSI2\", \\" >> cmd
echo "     \"vsi3.m5spec\" using (\$1+1024):2 with lines title \"VSI3\", \\" >> cmd
echo "     \"vsi4.m5spec\" using (\$1+1536):2 with lines title \"VSI4\" " >> cmd
echo "set logscale y; replot; pause -1" >> cmd
gnuplot cmd

