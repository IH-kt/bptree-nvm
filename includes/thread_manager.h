#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H
#  ifdef __cplusplus
extern "C" {
#  endif
#  include <pthread.h>
#  include <immintrin.h>
#  include <semaphore.h>
#  include "fptree.h"
#  include <stdlib.h>
#  include <errno.h>
#  include <stdio.h>

#  define BPTREE_BLOCK 1
#  define BPTREE_NON_BLOCK 2

struct BPTreeFunctionContainer {
    void *(* function)(BPTree *, void *);
    BPTree *bpt;
    sem_t *sem;
    void *retval;
    void *arg;
};

typedef struct BPTreeFunctionContainer BPTreeFunctionContainer;

void bptreeThreadInit(unsigned int flag);
void bptreeThreadDestroy();
pthread_t bptreeCreateThread(BPTree *, void *(*)(BPTree *, void *), void *);
void bptreeStartThread(void);
void bptreeWaitThread(pthread_t, void **);
#  ifdef __cplusplus
};
#  endif
#endif
