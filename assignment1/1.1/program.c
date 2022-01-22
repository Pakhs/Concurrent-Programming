#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include "mypipe.h"

#define FILE_INPUT "bufferIN"
#define FILE_OUTPUT "bufferOUT"
#define FILE_OUTPUT_SEC "bufferOUT_2"

volatile int wait = 1;

void *RtoW(void *args) {
    int *pipePos = (int *) args;
    int fd;
    char *c = (char *) malloc(sizeof(char));
    
    int items;
    char buffer[64];
    int i;
    
    fd = open(FILE_OUTPUT, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    
    while (pipe_read(*pipePos, c) == 1) {
        write(fd, c, 1);
    }
    close(fd);
    free(c);
    
    *pipePos = -1; // not a valid pos
    *pipePos = pipe_open(PIPE_SIZE);
    
    fd = open(FILE_OUTPUT, O_RDONLY);
    while ((items = read(fd,buffer, 64)) > 0) {
        for(i = 0; i < items;) {
            while(i < items) {
                pipe_write(*pipePos,buffer[i++]);
            }
        }
    }
    pipe_writeDone(*pipePos);
    close(fd);
    
    return (void *)NULL;
}

void *WtoR(void *args) {
    int *pipePos = (int *) args;
    char buffer[64];
    int i, items, fd;
    
    char *c = (char *) malloc(sizeof(char));
    
    fd = open(FILE_INPUT, O_RDONLY);
    while ((items = read(fd,buffer, 64)) > 0) {
        for(i = 0; i < items;) {
            while(i < items) {
                pipe_write(*pipePos,buffer[i++]);
            }
        }
    }
    pipe_writeDone(*pipePos);
    close(fd);
    
    while (*pipePos != -1);
    while (*pipePos == -1);
    
    fd = open(FILE_OUTPUT_SEC, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    
    while (pipe_read(*pipePos, c) == 1) {
        write(fd, c, 1);
    }
    close(fd);
    free(c);
    
    wait = 0;
    
    return (void *)NULL;
}

int main(int argc, char *argv[]) {
    int *pipePos = malloc(sizeof(int));
    *pipePos = pipe_open(PIPE_SIZE);
    
    pthread_t wThread, rThread;
    
    pthread_create(&wThread, NULL, WtoR, (void *)pipePos);
    pthread_create(&rThread, NULL, RtoW, (void *)pipePos);
    
    while(wait);
    free(pipePos);
    //free(pipeArray);
    
    return 0;
}