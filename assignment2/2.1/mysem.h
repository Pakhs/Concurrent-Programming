#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <errno.h>

struct mysem_t {
    int sem;
    bool valid;
};

extern int mysem_init(struct mysem_t *s, int n);
extern int mysem_down(struct mysem_t *s);
extern int mysem_up(struct mysem_t *s);
extern int mysem_destroy(struct mysem_t *s);