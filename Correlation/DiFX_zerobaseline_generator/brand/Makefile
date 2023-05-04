
# d262

all: clean prep run chk

.NOTPARALLEL:

clean:
	rm -rf brand_?.*

prep:
	./generate_zerobaseline_vex.sh /data/TESTS/brand-2023/framesize-7200/BRAND?.vdif brand

run:
	startdifx -f -A difxslurm brand_1.input 

chk:
	rm -rf 1234
	difx2mark4 -e 1234 -d
	fourfit -m1 -ptx 1234

manual:
	rm -rf 1234 *.difx *.input
	nano brand.v2d
	vex2difx brand.v2d
	startdifx -f -A difxslurm brand_1.input 

