#include "externalMerge.h"


int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "format: ./main <input file>\n");
        exit(-1);
    }
    ext_merge(argv[1]);

    return 0;
}