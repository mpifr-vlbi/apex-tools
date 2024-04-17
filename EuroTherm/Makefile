

all: eurotherm_test queryEurothermUDP

eurotherm_test: eurotherm_test.cpp
	gcc -Wall $< -o eurotherm_test

queryEurothermUDP: queryEurothermUDP.c
	gcc -Wall $< -o queryEurothermUDP -lm

clean:
	rm -f *.o queryEurothermUDP eurotherm_test
