CFLAGS = -Wall -g

all: program

program: program.c mypipe.o
	gcc $(CFLAGS) mypipe.o program.c -o program -pthread

mypipe.o: mypipe.c mypipe.h
	gcc $(CFLAGS) -c mypipe.c

diff: bufferIN bufferOUT bufferOUT_2
	diff bufferIN bufferOUT
	diff bufferOUT bufferOUT_2
	diff bufferIN bufferOUT_2

buf_rmv: bufferOUT bufferOUT_2
	rm bufferOUT
	rm bufferOUT_2

clean:
	rm mypipe.o program