if [ "$1" == "" ]; then
	echo "post_corr.sh <file.difx>"
else
	rm -rf 1234
	difx2mark4 -e 1234 $1
	fourfit -m1 1234
	# fplot 1234
	fplot -d out.ps 1234/No0001/aC.B.1.*
	echo "Generated plot in out.ps"
fi

