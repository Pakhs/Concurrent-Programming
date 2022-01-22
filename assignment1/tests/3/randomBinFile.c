#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char const *argv[]) {
    
    if (argc != 3) {
        printf("Correct usage: ./random.run <numOfIntegers> <max>\n");
        exit(-1);
    }
    int num = atoi(argv[1]);
    int max = atoi(argv[2]);
    srand(time(0));

    int i;
    int *n = malloc(sizeof(int));
    int f = open("out.bin", O_RDWR | O_CREAT | O_TRUNC, 0666);
    for (i = 0; i < num; ++i) {
        *n = rand() % (max + 1);
        write(f, n, sizeof(int));
    }
    close(f);
    
    return 0;
}
