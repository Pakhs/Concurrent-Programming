CFLAGS = -Wall -g -pthread

all: program

program: primeThreads.c
	gcc $(CFLAGS) primeThreads.c -o primeThreads -lm

clean:
	rm primeThreads