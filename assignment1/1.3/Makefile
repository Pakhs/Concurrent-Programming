CFLAGS = -Wall -g

all: main binary

main: main.c externalMerge.o
	gcc $(CFLAGS) externalMerge.o main.c -o main -pthread

externalMerge.o: externalMerge.c externalMerge.h
	gcc $(CFLAGS) -c externalMerge.c

clean:
	rm *.o *.bin *.run main

binary:
	gcc $(CFLAGS) ../tests/3/randomBinFile.c -o random.run
	gcc $(CFLAGS) ../tests/3/reverseBinFile.c -o reverse.run