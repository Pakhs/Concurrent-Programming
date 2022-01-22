#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
int main(int argc, char const *argv[]) {
    
    if (argc != 2) {
        printf("Correct usage: ./reverse.run <numOfIntegers>\n");
        exit(-1);
    }
    int num = atoi(argv[1]);
    
    int i;
    int f = open("in.bin", O_RDWR | O_CREAT | O_TRUNC, 0666);
    for (i = num; i > 0; i--) {
        write(f, &i, sizeof(int));
    }
    close(f);
    
    return 0;
}
