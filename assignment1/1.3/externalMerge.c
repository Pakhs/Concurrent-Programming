#include "externalMerge.h"

/*
 * 1 available
 * 0 working
*/

int minOfArray(int a[], int count[], int n) {
    int i, pos = -1, min = __INT_MAX__;

    for (i = 0; i < n; i++) {
        if (count[i] < PACK_SIZE) {
            if (a[i] <= min) {
                pos = i;
                min = a[i];
            }
        }
    }
    return pos;
}

int all_finished(int count[], int n) {
    int i;
    
    for (i = 0; i < n && count[i] == PACK_SIZE; i++);
    
    return (i < n) ? 0 : 1;
}

void packMerge(char filename[], int batches) {
    int out = open(FILE_OUT, O_RDWR | O_CREAT | O_TRUNC, 0666);
    int *f = (int *) malloc(batches * sizeof(int));
    int *buf = (int *) calloc(batches, sizeof(int));
    int *count = (int *) calloc(batches, sizeof(int));
    int i, pos;

    for (i = 0; i < batches; i++) {
        f[i] = open(filename, O_RDWR);
        lseek(f[i], (off_t) i*PACK_SIZE_INT, SEEK_SET);
    }
    
    for (i = 0; i < batches; i++)
        read(f[i], &buf[i], sizeof(int));
    
    while (!all_finished(count, batches)) {
        pos = minOfArray(buf, count, batches);
        write(out, &buf[pos], sizeof(int));
        count[pos]++;
        
        if (pos != batches - 1)
            read(f[pos], &buf[pos], sizeof(int));
        else {
            if (read(f[pos], &buf[pos], sizeof(int)) != sizeof(int))
                count[pos] = PACK_SIZE;
        }
    }
    
    for (i = 0; i < batches; i++) close(f[i]);
    
    close(out);
    
    free(f);
    free(buf);
    free(count);
}

void merge(volatile int array[], int start, int end, int mid) {
    int left_size = mid - start + 1;
    int right_size =  end - mid; 
    int *left = malloc(left_size * sizeof(int));
    int *right = malloc(right_size * sizeof(int));
    int i, j, k;
    
    for (i = 0; i < left_size; ++i) left[i] = array[i + start];
    for (i = 0; i < right_size; ++i) right[i] = array[i + mid + 1];
    
    k = start;
    i = j = 0;
    
    while (i < left_size && j < right_size) {
        if (left[i] <= right[j]) array[k++] = left[i++];
        else array[k++] = right[j++];
    }
    
    while (i < left_size) array[k++] = left[i++];
    while (j < right_size) array[k++] = right[j++];
    
    free(left);
    free(right);
}

void merge_sort(volatile int array[], int start, int end) {
    int mid = start + (end - start) / 2;
    
    if (start < end) {
        merge_sort(array, start, mid);
        merge_sort(array, mid + 1, end);
        merge(array, start, end, mid);
    }
}

void childInit(struct _args_ *father, struct _args_ *child, int pos) {
    child->buffer = &(father->buffer[pos]);
    
    if (! pos)
        child->numItems = father->numItems / 2;
    else
        child->numItems = father->numItems - (father->numItems / 2);
    
    child->father = father;
    child->comm = 1;
}

void *func(void *args) {
    struct _args_ *father = (struct _args_ *) args;
    struct _args_ *child1, *child2;
    pthread_t pid1, pid2;
    
    if (father->numItems <= 64) { // an ta stoixeia pou mas edwse o pateras einai <=64 kane mergeSort
        while (father->comm == 1);
        
        merge_sort(father->buffer, 0, father->numItems - 1);
        
        if (! father->father) father->comm = 1;
        else father->comm = -2; // teleiwsa me to sorting ws paidi, sou epistrefw sortarismeno pinaka
        
        return (void *) NULL;
    }
    
    while (father->comm != -1) {
        while (father->comm != 0); // perimene mexri na dw8ei entolh na trekseis
        
        child1 = (struct _args_ *) malloc(sizeof(struct _args_));
        childInit(father, child1, 0);

        pthread_create(&pid1, NULL, func, (void *) child1);
        
        child2 = (struct _args_ *) malloc(sizeof(struct _args_));
        childInit(father, child2, father->numItems / 2/*father->numItems - (father->numItems / 2)*/);
        pthread_create(&pid2, NULL, func, (void *) child2);
        
        child1->comm = 0;
        child2->comm = 0;
        
        while (child1->comm != -2 || child2->comm != -2); // perimene na teleiwsoun ta paidia
        
        free(child1);
        free(child2);
        
        merge(father->buffer, 0, father->numItems - 1, (father->numItems / 2) - 1);
        
        father->comm = -2;
        
        if (! father->father) {
            father->comm = 1;
        }
        
        return (void *) NULL;
    }
    
    
    return (void *)NULL;
}
void ext_merge(char filename[]) {
    struct _args_ *args = (struct _args_ *) malloc(sizeof(struct _args_));
    pthread_t pid;
    int f = open(filename, O_RDWR);
    int size = lseek(f, (off_t)0L, SEEK_END);
    int where = lseek(f, (off_t)0L, SEEK_SET);
    int batches = 0;
    int *buf = (int *) malloc(PACK_SIZE_INT);
    
    while (size - where >= PACK_SIZE_INT) {
        read(f, buf, PACK_SIZE_INT);

        args->buffer = buf;
        args->numItems = PACK_SIZE;
        args->comm = 0;
        args->father = NULL;
        
        pthread_create(&pid, NULL, func, (void *) args);
        while(args->comm != 1);

        lseek(f, (off_t) batches*PACK_SIZE_INT, SEEK_SET);
        write(f, buf, PACK_SIZE_INT);
        where = lseek(f, (off_t)0L, SEEK_CUR);
        batches += 1;
    }
    
    if (size - where) {
        read(f, buf, size - where);
        
        args->buffer = buf;
        args->numItems = (size - where) / sizeof(int);
        args->comm = 0;
        args->father = NULL;
        
        pthread_create(&pid, NULL, func, (void *) args);
        while(args->comm != 1);
        lseek(f, (off_t) batches*PACK_SIZE_INT, SEEK_SET);
        write(f, buf, size - where);
        where = lseek(f, (off_t)0L, SEEK_SET);
        batches += 1;
    }
    
    close(f);
    free(args);
    free(buf);
    packMerge(filename, batches);
}
