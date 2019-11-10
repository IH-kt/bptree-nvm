#include "fptree.h"
#include "thread_manager.h"
#include <stdlib.h>
#include <time.h>

int loop_times = 40;
int max_val = 1000;
int thread_max = 10;

typedef struct arg_t {
	unsigned int seed;
	unsigned char tid;
} arg_t;

void *insert_random(BPTree *bpt, void *arg) {
    arg_t *arg_cast = (arg_t *)arg;
    unsigned char tid = arg_cast->tid;
    KeyValuePair kv;
    unsigned int seed = arg_cast->seed;
    kv.key = 1;
    kv.value = 1;
    for (int i = 1; i <= loop_times; i++) {
        kv.key = rand_r(&seed) % max_val + 1;
        // printf("inserting %ld\n", kv.key);
        if (!insert(bpt, kv, tid)) {
            // fprintf(stderr, "insert: failure\n");
        }
        // showTree(bpt);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t *tid_array;
    struct timespec stt, edt;
    int i;
    BPTree *bpt = newBPTree();
    KeyValuePair kv;
    if (argc > 3) {
        loop_times = atoi(argv[1]);
        if (loop_times <= 0) {
            printf("invalid argument\n");
            return 1;
        }
        max_val = atoi(argv[2]);
        if (max_val <= 0) {
            printf("invalid argument\n");
            return 1;
        }
        thread_max = atoi(argv[3]);
        if (thread_max <= 0) {
            printf("invalid argument\n");
            return 1;
        }
        fprintf(stderr, "loop_times = %d, max_val = %d, thread_max = %d\n", loop_times, max_val, thread_max);
    } else {
        fprintf(stderr, "default: loop_times = %d, max_val = %d, thread_max = %d\n", loop_times, max_val, thread_max);
    }

    tid_array = (pthread_t *)malloc(sizeof(pthread_t) * thread_max);

    bptreeThreadInit(BPTREE_BLOCK);

    arg_t *arg = NULL;
    for (i = 0; i < thread_max; i++) {
        arg = (arg_t *)malloc(sizeof(int));
        arg->seed = i;
	arg->tid = i % 256 + 1;
        tid_array[i] = bptreeCreateThread(bpt, insert_random, arg);
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &stt);
    bptreeStartThread();

    for (i = 0; i < thread_max; i++) {
        bptreeWaitThread(tid_array[i], (void **)&arg);
        free(arg);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &edt);

    bptreeThreadDestroy();

    fprintf(stderr, "finish running threads\n");

    double time = edt.tv_nsec - stt.tv_nsec;
    time /= 1000000000;
    time += edt.tv_sec - stt.tv_sec;
    printf("%lf\n", time);

    // showTree(bpt);

    return 0;
}
