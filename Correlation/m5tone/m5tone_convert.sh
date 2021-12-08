#!/bin/bash
#
# Convert the mixed text .m5tone files ino numerical-columns-only .tone files
#

for fn in `ls -1 *.vdif.m5t`; do
	fout=${fn/.vdif.m5t/.tone}
	echo $fn $fout
	cat $fn | cut -c 1-15,22-33,40-51,55-64,70-75 > $fout
done
