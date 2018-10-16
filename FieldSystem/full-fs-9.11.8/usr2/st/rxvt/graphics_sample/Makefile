CFLAGS = -O2  

CC = gcc
all: qplot

clean:
	rm -f *.o 
	rm -f *.a 
	rm -f */*.o 
	rm -f */*.a
	rm -f a.out
	rm -f *~
	rm -f */*~ 
	rm -f #*# 
	rm -f */#*# 

OBJ = qplot.o graphics_lib.o

qplot: $(OBJ)
	$(CC) $(CFLAGS) -o qplot $(OBJ) -lm

