#ifndef __EXTMERGE_H
#define __EXTMERGE_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

#define PACK_SIZE 4096
#define PACK_SIZE_INT (PACK_SIZE * sizeof(int))
#define FILE_OUT "merged.bin"

struct _args_ {
    volatile int *buffer;
    int numItems;
    int comm;
    struct _args_ *father;
};

int minOfArray(int a[], int count[], int n);
int all_finished(int count[], int n);
void packMerge(char filename[], int batches);
void merge(volatile int array[], int start, int end, int mid);
void merge_sort(volatile int array[], int start, int end);
void childInit(struct _args_ *father, struct _args_ *child, int pos);
void *func(void *args);
extern void ext_merge(char filename[]);

#endif