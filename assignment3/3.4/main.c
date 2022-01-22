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

#define MAX_PERSONS 5
#define TIME_INSIDE 3

char menc[] = "\033[0;34m";
char end[] = "\033[0m";
char change[] = "\033[0;33m";
char womenc[] = "\033[0;35m";
char enter[] = "\033[0;32m";
char exited[] = "\033[0;31m";
char totalc[] = "\033[0;36m";
char ended[] = "\033[0;37m";

pthread_mutex_t mtx;
pthread_cond_t women, men, mainEnd;
int ret = -2;
int occupation = -1; //0 for women, 1 for men
int waitingMen = 0, waitingWomen = 0;
int peopleInWC = 0;

void womanEnter() {
    pthread_mutex_lock(&mtx);
    ++waitingWomen;

    while (peopleInWC == MAX_PERSONS || occupation) pthread_cond_wait(&women, &mtx);
    ++peopleInWC;
    --waitingWomen;
    if (peopleInWC < MAX_PERSONS) pthread_cond_signal(&women);
    pthread_mutex_unlock(&mtx);
}

void womanExit() {
    pthread_mutex_lock(&mtx);
    --peopleInWC;
    if (!peopleInWC && !waitingWomen && waitingMen > 0) {
        occupation = 1;
        printf("%sNo %swomen %swaiting, giving WC to %smen%s\n", change, womenc, change, menc, end);
        pthread_cond_signal(&men);
    }
    else if (!peopleInWC && waitingWomen > 0)
        pthread_cond_signal(&women);
    else if (!peopleInWC && !waitingMen && !waitingWomen && ret == -1) {
        pthread_cond_signal(&mainEnd);
    }
    else if (!peopleInWC && !waitingMen && !waitingWomen) {
        printf("%sNo people waiting, giving to whoever comes first%s\n", change, end);
        occupation = -1;
    }
    pthread_mutex_unlock(&mtx);
}

void *womanThread(void *args) {
    if (occupation == -1) occupation = 0; 
    womanEnter();

    printf("%sWoman %ld%s entered the WC %s(%d total)%s\n", womenc, pthread_self() % 1000, enter, totalc, peopleInWC, end);
    sleep(TIME_INSIDE);
    printf("%sWoman %ld%s left the WC%s\n", womenc, pthread_self() % 1000, exited, end);

    womanExit();
    return (void *) NULL;
}

void manEnter() {
    pthread_mutex_lock(&mtx);
    ++waitingMen;

    while (peopleInWC == MAX_PERSONS || !occupation) pthread_cond_wait(&men, &mtx);
    ++peopleInWC;
    --waitingMen;
    if (peopleInWC < MAX_PERSONS) pthread_cond_signal(&men);
    pthread_mutex_unlock(&mtx);
}

void manExit() {
    pthread_mutex_lock(&mtx);
    --peopleInWC;
    if (!peopleInWC && waitingWomen > 0) {
        occupation = 0;
        printf("%sWomen %swaiting, giving WC to them%s\n", womenc, change, end);
        pthread_cond_signal(&women);
    }
    else if (!peopleInWC && waitingMen > 0 && !waitingWomen)
        pthread_cond_signal(&men);
    else if (!peopleInWC && !waitingMen && !waitingWomen && ret == -1) {
        pthread_cond_signal(&mainEnd);
    }
    else if (!peopleInWC && !waitingMen && !waitingWomen) {
        printf("%sNo people waiting, giving to whoever comes first%s\n", change, end);
        occupation = -1;
    }
    pthread_mutex_unlock(&mtx);
}

void *manThread(void *args) {
    if (occupation == -1) occupation = 1;
    manEnter();

    printf("%sMan %ld%s entered the WC %s(%d total)%s\n", menc, pthread_self() % 1000, enter, totalc, peopleInWC, end);
    sleep(TIME_INSIDE);
    printf("%sMan %ld%s left the WC%s\n", menc, pthread_self() % 1000, exited, end);

    manExit();
    return (void *) NULL;
}


int main(int argc, char *argv[]) {
    pthread_t *pid = malloc(sizeof(pthread_t));
    char side;
    int num, delay, i, total = 0;
    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&women, NULL);
    pthread_cond_init(&men, NULL);
    pthread_cond_init(&mainEnd, NULL);
    
    FILE *f = fopen(argv[1], "r");
    
    while(1) {
        ret = fscanf(f, "%c %d %d\n", &side, &num, &delay);
        if (ret == -1) break;
        total += num;
        pid = realloc(pid, total * sizeof(pthread_t));
        if (side == 'W') {
            for (i = total - num; i < total; ++i) {
                pthread_create(&pid[i], NULL, womanThread, NULL);
            }
        }
        else if (side == 'M') {
            for (i = total - num; i < total; ++i) {
                pthread_create(&pid[i], NULL, manThread, NULL);
            }
        }
        sleep(delay);
    }
    fclose(f);
    
    printf("%s\n=-=-=-=-=-=-=-=-=-=-=-=\nInput ended, waiting all people to finish\n=-=-=-=-=-=-=-=-=-=-=-=\n\n%s", ended, end);
    pthread_cond_wait(&mainEnd, &mtx);
    printf("%s\n=-=-=-=-=-=-=-=-=-=-=-=\nNo people left, exiting\n=-=-=-=-=-=-=-=-=-=-=-=\n\n%s", ended, end);
    free(pid);
    
    return 0;
}