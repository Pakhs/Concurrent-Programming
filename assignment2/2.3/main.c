#include "../assignment1/mysem.h"
#include <unistd.h>
#include <fcntl.h>
#define MAX_CARS 5
#define TIME_TO_PASS 5

char leftc[] = "\033[0;31m";
char rightc[] = "\033[0;36m";
char end[] = "\033[0m";
char create[] = "\033[0;33m";
char ended[] = "\033[0;35m";
char give[] = "\033[0;32m";

struct mysem_t mtx, mainEnd;
struct mysem_t left, right;

int carsOnBridge = 0;
int waitingLeft = 0, waitingRight = 0;
int direction = -1; //0 left to right, 1 right to left, -1 no direction
int ret = -2;

void *car_left(void *args) {
    ++waitingLeft;
    mysem_down(&left);
    mysem_down(&mtx);

    if (direction) { //not my direction
        mysem_up(&mtx);
        mysem_down(&left);
    }

    if (carsOnBridge == MAX_CARS) mysem_down(&left);
    if (carsOnBridge < MAX_CARS - 1 && waitingLeft > 0) {
        sleep(1);
        mysem_up(&left);
    }
    
    --waitingLeft;
    ++carsOnBridge;
    mysem_up(&mtx);

    printf("%sCar %ld %sentered the bridge %sfrom left side%s\n", leftc, pthread_self() % 1000, create, leftc, end);
    sleep(TIME_TO_PASS);
    printf("%sCar %ld %sleft the bridge%s\n", leftc, pthread_self() % 1000, ended, end);

    mysem_down(&mtx);
    --carsOnBridge;
    if (!carsOnBridge && waitingRight > 0) {
        printf("%sGiving bridge to %sright %sside %s\n", give, rightc, give, end);
        direction = 1;
        mysem_up(&right);
    }
    else if (!carsOnBridge && waitingLeft > 0) mysem_up(&left);
    else if (!carsOnBridge && waitingLeft == 0) {
        printf("%sNo cars waiting, giving bridge to whoever comes first%s\n", give, end);
        direction = -1;
        mysem_up(&right);
        mysem_up(&left);
    }
    mysem_up(&mtx);
    if (waitingLeft == 0 && waitingRight == 0 && ret == -1) mysem_up(&mainEnd);
    
    return (void *) NULL;
}

void *car_right(void *args) {
    ++waitingRight;
    mysem_down(&right);
    mysem_down(&mtx);

    if (!direction) { //not my direction
        mysem_up(&mtx);
        mysem_down(&right);
    }
    if (carsOnBridge == MAX_CARS) mysem_down(&right);
    if (carsOnBridge < MAX_CARS - 1 && waitingRight > 0) {
        sleep(1);
        mysem_up(&right);
    }
    --waitingRight;
    ++carsOnBridge;
    mysem_up(&mtx);

    printf("%sCar %ld %sentered the bridge %sfrom right side%s\n", rightc, pthread_self() % 1000, create, rightc, end);
    sleep(TIME_TO_PASS);
    printf("%sCar %ld %sleft the bridge%s\n", rightc, pthread_self() % 1000, ended, end);

    mysem_down(&mtx);
    --carsOnBridge;
    if (!carsOnBridge && waitingLeft > 0) {
        direction = 0;
        mysem_up(&left);
    }
    else if (!carsOnBridge && waitingRight > 0) mysem_up(&right);
    else if (!carsOnBridge && waitingRight == 0) {
        printf("%sNo cars waiting, giving bridge to whoever comes first%s\n", give, end);
        direction = -1;
        mysem_up(&right);
        mysem_up(&left);
    }
        mysem_up(&mtx);
    if (waitingLeft == 0 && waitingRight == 0 && ret == -1) mysem_up(&mainEnd);
    
    return (void *) NULL;
}

int main(int argc, char *argv[]) {
    pthread_t *pid = malloc(sizeof(pthread_t));
    char side;
    int num, delay, i, total = 0;

    FILE *f = fopen(argv[1], "r");

    mtx.valid = false;
    left.valid = false;
    right.valid = false;
    mainEnd.valid = false;

    mysem_init(&mtx, 1);
    mysem_init(&left, 0);
    mysem_init(&right, 0);
    mysem_init(&mainEnd, 0);

    while(1) {
        ret = fscanf(f, "%c %d %d\n", &side, &num, &delay);
        if (ret == -1) break;
        total += num;
        pid = realloc(pid, total * sizeof(pthread_t));
        if (side == 'L') {
            if (direction == -1) {
                direction = 0;
                mysem_up(&left);
            }
            for (i = total - num; i < total; ++i)
                pthread_create(&pid[i], NULL, car_left, NULL);
        }
        else if (side == 'R') {
            if (direction == -1) {
                direction = 1;
                mysem_up(&right);
            }
            for (i = total - num; i < total; ++i)
                pthread_create(&pid[i], NULL, car_right, NULL);
        }
        sleep(delay);
    }

    fclose(f);

    printf("%s\n=-=-=-=-=-=-=-=-=-=-=-=\nInput ended, waiting all cars to finish\n=-=-=-=-=-=-=-=-=-=-=-=\n\n%s", ended, end);
    mysem_down(&mainEnd);
    sleep(TIME_TO_PASS);
    printf("%s\n=-=-=-=-=-=-=-=-=-=-=-=\nNo cars left, exiting\n=-=-=-=-=-=-=-=-=-=-=-=\n\n%s", ended, end);

    free(pid);
    
    return 0;
}