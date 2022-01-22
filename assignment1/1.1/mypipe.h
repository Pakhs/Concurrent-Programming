#ifndef __MYPIPE_H
#define __MYPIPE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PIPE_SIZE 10

struct _pipe_ {
    char *buffer;
    char *w;
    char *r;
    volatile int itmCount;
    volatile int turn; // 0 write, 1 read
    int wantW;
    int wantR;
    int writeDone;
	int pipeSize;
};

extern int pipe_open(int size);

extern int pipe_writeDone(int p);

extern int pipe_read(int p, char *c);

extern int pipe_write(int p, char c);

#endif