CFLAGS = -Wall -g -std=c99

all: main

main: main.c mysem.o
	gcc $(CFLAGS) mysem.o main.c -o main -pthread

mysem.o: ../assignment1/mysem.c 
	gcc $(CFLAGS) -c ../assignment1/mysem.c

rmv:
	rm *.o main