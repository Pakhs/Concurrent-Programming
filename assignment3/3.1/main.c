#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>

bool isThereInput = false;
char threadc[] = "\033[0;32m";
char numc[] = "\033[0;33m";
char end[] = "\033[0m";

struct threadStruct {
    bool mainDone;
    bool avail;
    int input;
    int numWorkers;
    pthread_mutex_t *mtx;
    pthread_cond_t *condAvail;
    pthread_cond_t *condRead;
    pthread_cond_t *condWait;
    pthread_cond_t *condFinish;
};

int isPrime(int n) {
    int i;
    
    for (i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    
    return (n <= 1) ? 0 : 1;
}

void *func(void *args) {
    struct threadStruct *tInput = (struct threadStruct *) args;
    
    while (1) {
        pthread_mutex_lock(tInput->mtx);
        
        pthread_cond_signal(tInput->condAvail);
        tInput->avail = true;
        
        pthread_cond_wait(tInput->condRead, tInput->mtx);
        
        tInput->avail = false;
        
        if (tInput->mainDone) break;
        
        if (isPrime(tInput->input)) {
            printf("%s[ %ld ]%s %d is prime\n%s", threadc, pthread_self() % 1000, numc,  tInput->input, end);
        }
        else {
            printf("%s[ %ld ]%s %d is not prime\n%s", threadc, pthread_self() % 1000, numc,  tInput->input, end);
        }
        
        pthread_cond_signal(tInput->condWait);
        pthread_mutex_unlock(tInput->mtx);
    }
    
    pthread_cond_signal(tInput->condFinish);
    pthread_mutex_unlock(tInput->mtx);
    
    return (void *) NULL;
}

int main(int argc, char *argv[]) {
    pthread_t *pid;
    struct threadStruct tInput;
    int i;
    
    if (argc != 2) {
        fprintf(stderr, "format: ./primeThreads <number of threads>\n");
        exit(-1);
    }
    
    tInput.numWorkers = atoi(argv[1]);
    if (!tInput.numWorkers) exit(-1);
    
    tInput.mainDone = false;
    tInput.avail = false;
    
    tInput.mtx = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    tInput.condAvail = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    tInput.condRead = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    tInput.condWait = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    tInput.condFinish = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    
    pthread_mutex_init(tInput.mtx, NULL);
    pthread_cond_init(tInput.condAvail, NULL);
    pthread_cond_init(tInput.condRead, NULL);
    pthread_cond_init(tInput.condWait, NULL);
    pthread_cond_init(tInput.condFinish, NULL);
    
    pid = (pthread_t *) malloc(tInput.numWorkers * sizeof(pthread_t));
    for (i = 0; i < tInput.numWorkers; i++) {
        pthread_create(&pid[i], NULL, func, (void *) &tInput);
    }
    
    while(scanf("%d", &tInput.input) != EOF) {
        while (tInput.avail != true)
            pthread_cond_wait(tInput.condAvail, tInput.mtx);
        
        pthread_cond_signal(tInput.condRead);
        pthread_cond_wait(tInput.condWait, tInput.mtx);
    }
    
    tInput.mainDone = true;
    
    for(i = 0; i < tInput.numWorkers; i++) {
        pthread_cond_signal(tInput.condRead);
        pthread_cond_wait(tInput.condFinish, tInput.mtx);
    }
    
    return 0;
}