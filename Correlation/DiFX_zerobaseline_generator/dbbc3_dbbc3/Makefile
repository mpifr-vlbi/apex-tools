all:

clean:
	rm -rf 1234 test_1.*

full:
	for pct in 100 52 34 24 18 12 8 5 3.3 2.7 1.7 ; do \
	# for pct in 52 ; do \
		rm -rf test?_1.* 1234 test?.vex test?.v2d *.lst *.calc *.im ; \
		./generate_zerobaseline_0-2G_fullbw.sh \
			/data/TESTS/dbbc3_nov18/vdif/ZB_DBBC3_2_0-101218_$${pct}pc_a.vdif \
			/data/TESTS/dbbc3_nov18/vdif/ZB_DBBC3_2_0-101218_$${pct}pc_b.vdif \
			test ; \
		startdifx -f test_1.input ; \
		difx2mark4 -e 1234 -s codes *.difx ; \
		echo -n "$${pct} : " > ZB_DBBC3_2_0-101218_$${pct}pc_full2g.amplitudes.txt ; \
		fourfit -m1 -bRR -PLR 1234  2>&1 | grep "found amp" >>  ZB_DBBC3_2_0-101218_$${pct}pc_full2g.amplitudes.txt; \
		fplot2ps 1234/No0001 R R ; \
		mv 1234/No0001_R.pdf result.pdf ; \
		cp result.pdf ZB_DBBC3_2_0-101218_$${pct}pc_full2g.pdf ; \
	done

full128:
	for pct in 100 52 34 24 18 12 8 5 3.3 2.7 1.7 ; do \
	# for pct in 52 ; do \
		rm -rf test?_1.* 1234 test?.vex test?.v2d *.lst *.calc *.im ; \
		./generate_zerobaseline_0-2G.sh \
			/data/TESTS/dbbc3_nov18/vdif/ZB_DBBC3_2_0-101218_$${pct}pc_a.vdif \
			/data/TESTS/dbbc3_nov18/vdif/ZB_DBBC3_2_0-101218_$${pct}pc_b.vdif \
			test ; \
		startdifx -f test_1.input ; \
		difx2mark4 -e 1234 -s codes *.difx ; \
		echo -n "$${pct} : " > ZB_DBBC3_2_0-101218_$${pct}pc_first128M.amplitudes.txt ; \
		fourfit -m1 -bRR -PLR 1234  2>&1 | grep "found amp" >>  ZB_DBBC3_2_0-101218_$${pct}pc_first128M.amplitudes.txt; \
		fplot2ps 1234/No0001 R R ; \
		mv 1234/No0001_R.pdf result.pdf ; \
		cp result.pdf ZB_DBBC3_2_0-101218_$${pct}pc_first128M.pdf ; \
	done
