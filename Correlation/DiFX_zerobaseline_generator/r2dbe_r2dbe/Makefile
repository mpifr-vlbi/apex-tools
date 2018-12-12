none:

PCT=0.7

clean:
	rm -rf 1234 test_1.*

test:
	# Here stations are 2-pol but LCP=R2DBE if0 data, RCP=R2DBE if1 data
	# Auto-corr crosspol has actual cross-backend fringe (max amp 10,000)

	rm -rf 1234 test_1.*
	./generate_zerobaseline_0-2G_fullbw.sh \
		/data/TESTS/dbbc3_nov18/vdif/ZB_R2DBE_111218_$(PCT)pc_a.vdif \
		/data/TESTS/dbbc3_nov18/vdif/ZB_R2DBE_111218_$(PCT)pc_b.vdif \
		test
	startdifx -f test_1.input
	rm -rf 1234
	difx2mark4 -e 1234 -s codes *.difx
	fourfit -bRR -PLR 1234
	fplot2ps 1234/No0001 R R
	mv 1234/No0001_R.pdf result.pdf
	cp result.pdf ZB_R2DBE_111218_$(PCT)pc.pdf

full:
	for pct in 100 52 34 25 18 12 8 5 3.3 2.7 1.8 1.1 0.7 ; do \
		rm -rf 1234 test_1.* ; \
		./generate_zerobaseline_0-2G_fullbw.sh \
			/data/TESTS/dbbc3_nov18/vdif/ZB_R2DBE_111218_$${pct}pc_a.vdif \
			/data/TESTS/dbbc3_nov18/vdif/ZB_R2DBE_111218_$${pct}pc_b.vdif \
			test ; \
		startdifx -f test_1.input ; \
		difx2mark4 -e 1234 -s codes *.difx ; \
		echo -n "$${pct} : " > ZB_R2DBE_111218_$${pct}pc.amplitudes.txt ; \
		fourfit -m1 -bRR -PLR 1234   2>&1 | grep "found amp" >> ZB_R2DBE_111218_$${pct}pc.amplitudes.txt  ; \
		fplot2ps 1234/No0001 R R ; \
		mv 1234/No0001_R.pdf result.pdf ; \
		cp result.pdf ZB_R2DBE_111218_$${pct}pc.pdf ; \
	done
