#include "../assignment1/mysem.h"
#include <unistd.h>

#define TIMESLEEP 3

char numColor[] = "\033[0;33m";
char tColor[] = "\033[0;35m";
char pColor[] = "\033[0;36m";
char errorColor[] = "\033[0;31m";
char end[] = "\033[0m";

struct train {
    int numPassengers;
    int maxPassengers;
    struct mysem_t train;
    struct mysem_t passenger;
};

void checkParams(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%susage: ./main <maxPassengers>\n%s", errorColor, end);
        exit(-1);
    }
    if (atoi(argv[1]) <= 0) {
        fprintf(stderr, "%serror: maxPassengers must be a positive integer%s\n", errorColor, end);
        exit(-1);
    }
}

void *trainThread(void *args) {
    struct train *t = (struct train *) args;
    while (mysem_down(&(t->train))) {
        
        printf("%sTrain running with %s%d%s passengers \n%s",tColor, numColor, t->numPassengers, tColor, end);
        sleep(TIMESLEEP);
        printf("%sPassengers left the train%s\n\n", tColor, end);
        sleep(TIMESLEEP / 2);
        t->numPassengers = 0;

        mysem_up(&(t->passenger));
    }
    return (void *) NULL;
}
void *passengerThread(void *args) {
    struct train *t = (struct train *) args;
    // printf("passenger %ld created and waiting to be called\n", pthread_self() % 10000);
    mysem_down(&(t->passenger));
    printf("%sPassenger %s%ld%s entering train, %s%d%s total in train%s\n", pColor, numColor, pthread_self() % 1000, pColor, numColor, t->numPassengers + 1, pColor, end);
    t->numPassengers++;
    sleep(TIMESLEEP / 3);

    if (t->numPassengers == t->maxPassengers) mysem_up(&(t->train));
    else mysem_up(&(t->passenger));

    return (void *) NULL;
}

int main(int argc, char **argv) {
    struct train t;
    int passengers, i;
    pthread_t pidTrain;
    pthread_t *pidPassengers;
    int totalPassengers = 0;

    checkParams(argc, argv);
    t.maxPassengers = atoi(argv[1]);
    t.numPassengers = 0;
    
    t.train.valid = false;
    t.passenger.valid = false;
    mysem_init(&(t.train), 0);
    
    pthread_create(&pidTrain, NULL, trainThread, &t);
    pidPassengers = malloc(sizeof(pthread_t));
    mysem_init(&(t.passenger), 1);
    
    while(1) {
        scanf("%d", &passengers);
        totalPassengers += passengers;
        pidPassengers = (pthread_t *) realloc(pidPassengers, totalPassengers * sizeof(pthread_t));
        
        for (i = totalPassengers - passengers; i < totalPassengers; i++) {
            pthread_create(&pidPassengers[i], NULL, passengerThread, &t);
        }
    }

    
    return 0;
}