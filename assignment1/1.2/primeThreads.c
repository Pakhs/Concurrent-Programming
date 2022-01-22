#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#define N 5

/*
 * 1 not working
 * 0 processing
 * -1 signaled by main process to terminate
 * -2 signaled by worker process that it has terminated
*/

int isPrime(int n) {
    int i;
    
    for (i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
            return 0;
        }
    }
 
    if (n <= 1)
        return 0;
    
    return 1;
}

void *func(void *myComm) {
    int *arr = (int *) myComm;
    
    while (1) {
        if (! arr[0]) {
            if (isPrime(arr[1])) {
                printf("%d is prime\n", arr[1]);
            }
            else {
                printf("%d is not prime\n", arr[1]);
            }
            arr[0] = 1;
        }
        else if (arr[0] == -1) {
            break;
        }
    }
    
    arr[0] = -2;
    
    return (void *) NULL;
}

int main(int argc, char *argv[]) {
    int i, input, numWorkers, **comm;
    pthread_t *pid;
    
    if (argc != 2) {
        fprintf(stderr, "format: ./primeThreads <number of threads>\n");
        exit(-1);
    }
    
    numWorkers = atoi(argv[1]);
    
    if (!numWorkers) exit(-1);
    
    comm = (int **) malloc(numWorkers * sizeof(int *));
    pid = (pthread_t *) malloc(numWorkers * sizeof(pthread_t));
    
    for (i = 0; i < numWorkers; i++) {
        comm[i] = (int *) malloc(2 * sizeof(int));
        comm[i][0] = 1;
        pthread_create(&pid[i], /*&attr*/NULL, func, (void *) comm[i]);
    }
    
    while(scanf("%d", &input) != EOF) {
        do {
            for (i = 0; i < numWorkers && comm[i][0] == 0; i++);
        } while (i == numWorkers);
        
        comm[i][1] = input;
        comm[i][0] = 0;
    }
    
    do {
        for (i = 0; i < numWorkers && comm[i][0] == 1; i++);
    } while (i != numWorkers);
    
    for (i = 0; i < numWorkers; i++) {
        comm[i][0] = -1;
    }
    
    do {
        for (i = 0; i < numWorkers && comm[i][0] == -2; i++);
    } while (i != numWorkers);
    
    for (i = 0; i < numWorkers; i++) {
        free(comm[i]);
    }
    
    free(pid);
    free(comm);
    
    return 0;
}