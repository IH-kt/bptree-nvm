#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H
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
    void *(* function)(BPTree *);
    BPTree *bpt;
    sem_t *sem;
    void *retval;
};

typedef struct BPTreeFunctionContainer BPTreeFunctionContainer;

void bptreeThreadInit(unsigned int flag);
void bptreeThreadDestroy();
pthread_t bptreeCreateThread(BPTree *, void *(*)(BPTree *));
void bptreeStartThread(void);
void bptreeWaitThread(pthread_t, void **);
#endif
