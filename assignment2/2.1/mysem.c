#include "mysem.h"

int mysem_init(struct mysem_t *s, int n) {
    int semid;

    if (n < 0) return 0;
    if (s->valid) return -1;

    /* Create a new Semaphore */
    if ((semid = semget(IPC_PRIVATE, 1, S_IRWXU)) == -1) {
        fprintf(stderr, "In file %s, line %d ", __FILE__, __LINE__);
        perror("semget error");
        exit(1);
    }
    s->sem = semid;

    /* Set Semaphore value to n */
    if (semctl(s->sem, 0, SETVAL, n) == -1) {
        fprintf(stderr, "In file %s, line %d ", __FILE__, __LINE__);
        perror("semctl error");
        exit(1);
    }
    
    s->valid = true;
    
    return 1;
}

int mysem_down(struct mysem_t *s) {
    struct sembuf op;

    /* Check if Semaphore exists */
    if (! s->valid) return -1;

    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = 0;

    if (semop(s->sem, &op, 1) == -1) {
        fprintf(stderr, "In file %s, line %d ", __FILE__, __LINE__);
        perror("semop error");
        exit(1);
    }

    return 1;
}

int mysem_up(struct mysem_t *s) {
    int retVal;
    struct sembuf op;

    /* Check if Semaphore exists */
    if (! s->valid) return -1;
    
    retVal = semctl(s->sem, 0, GETVAL);

    if (retVal == -1) {
        fprintf(stderr, "In file %s, line %d ", __FILE__, __LINE__);
        perror("semctl error");
        exit(1);
    }
    else if (retVal == 1) {
        return 0;
    }

    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;

    if (semop(s->sem, &op, 1) == -1) {
        fprintf(stderr, "In file %s, line %d ", __FILE__, __LINE__);
        perror("semop error");
        exit(1);
    }

    return 1;
}

int mysem_destroy(struct mysem_t *s) {
    /* Check if Semaphore exists */
    if (! s->valid) return -1;
    
    if (semctl(s->sem, 0, IPC_RMID) == -1) {
        fprintf(stderr, "In file %s, line %d ", __FILE__, __LINE__);
        perror("semctl error");
        exit(1);
    }
    
    s->valid = false;
    
    return 1;
}
