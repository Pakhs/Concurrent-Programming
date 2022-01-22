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

#define MAX_CARS 5
#define TIME_TO_PASS 3

char leftc[] = "\033[0;31m";
char rightc[] = "\033[0;36m";
char end[] = "\033[0m";
char create[] = "\033[0;33m";
char ended[] = "\033[0;35m";
char give[] = "\033[0;32m";

pthread_mutex_t mtx;
pthread_cond_t left, right, mainEnd;
int ret = -2, direction = -1;
int waitingRight = 0, waitingLeft = 0;
int carsOnBridge = 0;

void left_enter() {
    pthread_mutex_lock(&mtx);
    ++waitingLeft;
    
    while (carsOnBridge == MAX_CARS || direction) pthread_cond_wait(&left, &mtx);
    ++carsOnBridge;
    --waitingLeft;
    if (carsOnBridge < MAX_CARS) pthread_cond_signal(&left);
    pthread_mutex_unlock(&mtx);
}

void left_exit() {
    pthread_mutex_lock(&mtx);
    --carsOnBridge;
    if (!carsOnBridge && waitingRight > 0) {
        direction = 1;
        printf("%sGiving bridge to %sright %sside %s\n", give, rightc, give, end);
        pthread_cond_signal(&right);
    }
    else if (!carsOnBridge && waitingLeft > 0) {
        printf("%sContinuing on %sleft %sside %s\n", give, leftc, give, end);
        pthread_cond_signal(&left);
    }
    else if (!carsOnBridge && !waitingLeft && !waitingRight && ret == -1) {
        pthread_cond_signal(&mainEnd);
    }
    else if (!carsOnBridge && !waitingLeft && !waitingRight) {
        printf("%sNo cars waiting, giving to whoever comes first%s\n", give, end);
        direction = -1;
    }
    pthread_mutex_unlock(&mtx);
}

void *car_left(void *args) {
    if (direction == -1) direction = 0;
    left_enter();
    
    printf("%sCar %ld %sentered the bridge %sfrom left side%s\n", leftc, pthread_self() % 1000, create, leftc, end);
    sleep(TIME_TO_PASS);
    printf("%sCar %ld %sleft the bridge%s\n", leftc, pthread_self() % 1000, ended, end);
    
    left_exit();
    return (void *) NULL;
}

void right_enter() {
    pthread_mutex_lock(&mtx);
    ++waitingRight;
    
    while (carsOnBridge == MAX_CARS || !direction) pthread_cond_wait(&right, &mtx);
    ++carsOnBridge;
    --waitingRight;
    if (carsOnBridge < MAX_CARS) pthread_cond_signal(&right);
    pthread_mutex_unlock(&mtx);
}

void right_exit() {
    pthread_mutex_lock(&mtx);
    --carsOnBridge;
    if (!carsOnBridge && waitingLeft > 0) {
        direction = 0;
        printf("%sGiving bridge to %sleft %sside %s\n", give, leftc, give, end);
        pthread_cond_signal(&left);
    }
    else if (!carsOnBridge && waitingRight > 0) {
        printf("%sContinuing on %sright %sside %s\n", give, rightc, give, end);
        pthread_cond_signal(&right);
    }
    else if (!carsOnBridge && !waitingLeft && !waitingRight && ret == -1) {
        pthread_cond_signal(&mainEnd);
    }
    else if (!carsOnBridge && !waitingLeft && !waitingRight) {
        printf("%sNo cars waiting, giving to whoever comes first%s\n", give, end);
        direction = -1;
    }
    pthread_mutex_unlock(&mtx);
}

void *car_right(void *args) {
    if (direction == -1) direction = 1;
    right_enter();
    
    printf("%sCar %ld %sentered the bridge %sfrom right side%s\n", rightc, pthread_self() % 1000, create, rightc, end);
    sleep(TIME_TO_PASS);
    printf("%sCar %ld %sleft the bridge%s\n", rightc, pthread_self() % 1000, ended, end);
    
    right_exit();
    return (void *) NULL;
}


int main(int argc, char *argv[]) {
    pthread_t *pid = malloc(sizeof(pthread_t));
    char side;
    int num, delay, i, total = 0;
    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&left, NULL);
    pthread_cond_init(&right, NULL);
    pthread_cond_init(&mainEnd, NULL);
    
    FILE *f = fopen(argv[1], "r");
    
    while(1) {
        ret = fscanf(f, "%c %d %d\n", &side, &num, &delay);
        if (ret == -1) break;
        total += num;
        pid = realloc(pid, total * sizeof(pthread_t));
        if (side == 'L') {
            for (i = total - num; i < total; ++i) {
                pthread_create(&pid[i], NULL, car_left, NULL);
            }
        }
        else if (side == 'R') {
            for (i = total - num; i < total; ++i) {
                pthread_create(&pid[i], NULL, car_right, NULL);
            }
        }
        sleep(delay);
    }
    fclose(f);
    
    printf("%s\n=-=-=-=-=-=-=-=-=-=-=-=\nInput ended, waiting all cars to finish\n=-=-=-=-=-=-=-=-=-=-=-=\n\n%s", ended, end);
    pthread_cond_wait(&mainEnd, &mtx);
    printf("%s\n=-=-=-=-=-=-=-=-=-=-=-=\nNo cars left, exiting\n=-=-=-=-=-=-=-=-=-=-=-=\n\n%s", ended, end);
    
    free(pid);
    
    return 0;
}