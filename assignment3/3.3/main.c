#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define TIME_TO_PASS 2

char numColor[] = "\033[0;33m";
char tColor[] = "\033[0;35m";
char pColor[] = "\033[0;36m";
char errorColor[] = "\033[0;31m";
char end[] = "\033[0m";

struct arguments {
    pthread_mutex_t mtx;
    pthread_cond_t train;
    pthread_cond_t passenger;
    pthread_cond_t pass_exit;
    pthread_cond_t exiting;
    int current_passengers;
    int max_passengers;
    int waitingToEnter;
    int waitingToExit;
    int isExiting;
};

void *passengerThread(void *args) {
    struct arguments *t = (struct arguments *) args; 
    
    //enter
    pthread_mutex_lock(&t->mtx);
    pthread_cond_wait(&t->passenger, &t->mtx);
    if (t->isExiting) {
        t->waitingToEnter++;
        pthread_cond_wait(&t->exiting, &t->mtx);
    }

    //main
    t->current_passengers++;
    printf("%sPassenger %s%ld%s entering train, %s%d%s total in train%s\n", pColor, numColor, pthread_self() % 1000, pColor, numColor, t->current_passengers, pColor, end);
    if (t->current_passengers == t->max_passengers) pthread_cond_signal(&t->train);
    pthread_cond_wait(&t->pass_exit, &t->mtx);
    pthread_mutex_unlock(&t->mtx);
    
    //exit
    pthread_mutex_lock(&t->mtx);
    t->current_passengers--;
    printf("%sPassenger %s%ld%s exited, %s%d%s total in train%s\n", tColor, numColor, pthread_self() % 1000, tColor, numColor, t->current_passengers, tColor, end);
    int i;
    if (t->current_passengers == 0) {
        sleep(1);
        t->isExiting = 0;
        for (i = 0; i < t->waitingToEnter; i++) {
            pthread_cond_signal(&t->exiting);
        }
        t->waitingToEnter = 0;
    }
    pthread_mutex_unlock(&t->mtx);
    
    return (void *) NULL;
}

void *trainThread(void *args) {
    struct arguments *t = (struct arguments *) args;
    int i;
    
    while (1) {
        for (i = 0; i < t->max_passengers; i++) pthread_cond_signal(&t->passenger);
        pthread_cond_wait(&t->train, &t->mtx);
        
        printf("\n%sTrain running with %s%d%s passengers \n\n%s",tColor, numColor, t->current_passengers, tColor, end);
        sleep(3);
        
        t->isExiting = 1;
        for (i = 0; i < t->current_passengers; i++) pthread_cond_signal(&t->pass_exit);
    }
}

int main(int argc, char const *argv[]) {
    struct arguments args;
    pthread_mutex_init(&args.mtx, NULL);
    pthread_cond_init(&args.train, NULL);
    pthread_cond_init(&args.passenger, NULL);
    pthread_cond_init(&args.pass_exit, NULL);
    pthread_cond_init(&args.exiting, NULL);
    
    int passengers, i;
    pthread_t pidTrain;
    pthread_t *pidPassengers;
    int totalPassengers = 0;
    
    args.max_passengers = atoi(argv[1]);
    args.current_passengers = 0;
    
    pidPassengers = malloc(sizeof(pthread_t));
    args.isExiting = 0;
    args.waitingToEnter = 0;
    args.waitingToExit = 0;
    
    while(1) {
    scanf("%d", &passengers);
        totalPassengers += passengers;
        pidPassengers = (pthread_t *) realloc(pidPassengers, totalPassengers * sizeof(pthread_t));
        
        for (i = totalPassengers - passengers; i < totalPassengers; i++) {
            pthread_create(&pidPassengers[i], NULL, passengerThread, &args);
        }
        pthread_create(&pidTrain, NULL, trainThread, &args);
    }
    
    return 0;
}
