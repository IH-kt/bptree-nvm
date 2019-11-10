#include "thread_manager.h"

__thread sem_t *sem = NULL;
__thread unsigned int number_of_thread = 0;

void *bptreeThreadFunctionWrapper(void *container_v) {
    BPTreeFunctionContainer *container = (BPTreeFunctionContainer *)container_v;
    if (container->sem != NULL) {
        if (sem_wait(container->sem) == -1) {
            perror("sem_wait");
        }
    }
    container->retval = container->function(container->bpt, container->arg);
    pthread_exit(container);
}

void bptreeThreadInit(unsigned int flag) {
    if (flag & BPTREE_BLOCK) {
        // init semaphore
        if (sem == NULL) {
            sem = (sem_t *)vmem_allocate(sizeof(sem_t));
            if (sem_init(sem, 0, 0) == -1) {
                perror("sem_init");
                exit(1);
            }
        } else {
            printf("initializer called multiple times\n");
            exit(2);
        }
    }
}

void bptreeThreadDestroy() {
    if (sem != NULL) {
        sem_destroy(sem);
        vmem_free(sem);
        sem = NULL;
    }
}

pthread_t bptreeCreateThread(BPTree *bpt, void *(* thread_function)(BPTree *, void *), void *arg) {
    pthread_t tid;
    BPTreeFunctionContainer *container = (BPTreeFunctionContainer *)vmem_allocate(sizeof(BPTreeFunctionContainer));
    container->function = thread_function;
    container->bpt = bpt;
    container->sem = sem;
    container->retval = NULL;
    container->arg = arg;
    number_of_thread++;
    if (pthread_create(&tid, NULL, bptreeThreadFunctionWrapper, container) == EAGAIN) {
        printf("pthread_create: reached resource limit\n");
    }
    return tid;
}

void bptreeStartThread(void) {
    if (sem != NULL) {
        int i;
        for (i = 0; i < number_of_thread; i++) {
            sem_post(sem);
        }
    }
}

void bptreeWaitThread(pthread_t tid, void **retval) {
    void *container_v;
    pthread_join(tid, &container_v);
    if (retval != NULL) {
        *retval = ((BPTreeFunctionContainer *)container_v)->retval;
    }
    vmem_free(container_v);
    number_of_thread--;
}
