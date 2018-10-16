FFLAGS= -m32
FFLAGS= -m32
CFLAGS = -m32  -Wall -O3 -s -fomit-frame-pointer -lm -lncurses -pthread
#
../bin/predict: predict.c
	cc $(CFLAGS) predict.c -o ../bin/predict
	strip ../bin/predict

predict:
	rm -f ../bin/predict
	make
#
