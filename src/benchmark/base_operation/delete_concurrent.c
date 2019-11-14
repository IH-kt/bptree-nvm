#ifdef NVHTM
#  include "nvhtm.h"
#endif
#include "fptree.h"
#include "thread_manager.h"
#include <stdlib.h>
#include <time.h>

int warm_up = 40;
int loop_times = 40;
int max_val = 1000;
int thread_max = 10;

typedef struct arg_t {
	unsigned int loop;
	unsigned int seed;
	unsigned char tid;
} arg_t;

void *delete_random(BPTree *bpt, void *arg) {
#ifdef NVHTM
    NVHTM_thr_init();
#endif
    arg_t *arg_cast = (arg_t *)arg;
    unsigned char tid = arg_cast->tid;
    Key key = 1;
    unsigned int seed = arg_cast->seed;
    unsigned int loop = arg_cast->loop;
    for (int i = 1; i <= loop; i++) {
        key = rand_r(&seed) % max_val + 1;
        bptreeRemove(bpt, key, tid);
        // showTree(bpt);
    }
#ifdef TIME_PART
    showTime(tid);
#endif
#ifdef NVHTM
    NVHTM_thr_exit();
#endif
    return NULL;
}

char const *pmem_path = "data";

int main(int argc, char *argv[]) {
    pthread_t *tid_array;
    struct timespec stt, edt;
    int i;
    BPTree *bpt;
    KeyValuePair kv;
    if (argc > 4) {
        warm_up = atoi(argv[1]);
        if (warm_up <= 0) {
            fprintf(stderr, "invalid argument\n");
            return 1;
        }
        loop_times = atoi(argv[2]);
        if (loop_times <= 0) {
            fprintf(stderr, "invalid argument\n");
            return 1;
        }
        max_val = atoi(argv[3]);
        if (max_val <= 0) {
            fprintf(stderr, "invalid argument\n");
            return 1;
        }
        thread_max = atoi(argv[4]);
        if (thread_max <= 0) {
            fprintf(stderr, "invalid argument\n");
            return 1;
        }
	pmem_path = argv[5];
        fprintf(stderr, "warm_up = %d, loop_times = %d, max_val = %d, thread_max = %d, pmem_path = %s\n", warm_up, loop_times, max_val, thread_max, pmem_path);
    } else {
        fprintf(stderr, "default: warm_up = %d, loop_times = %d, max_val = %d, thread_max = %d\n, pmem_path = %s", warm_up, loop_times, max_val, thread_max, pmem_path);
    }

    size_t allocation_size = sizeof(PersistentLeafNode) * loop_times / MAX_PAIR * 2;
#ifdef NVHTM
    NVHTM_init(thread_max);
    void *pool = NH_alloc(allocation_size);
    initAllocator(pool, pmem_path, allocation_size, thread_max);
    NVHTM_clear();
#else
    initAllocator(NULL, pmem_path, allocation_size, thread_max);
#endif
    bpt = newBPTree();

    kv.key = 1;
    kv.value = 1;
    unsigned int seed = thread_max;
    for (i = 0; i < warm_up; i++) {
	kv.key = rand_r(&seed) % max_val + 1;
        insert(bpt, kv, thread_max);
    }
    // showTree(bpt, 0);
#ifdef NVHTM
    NVHTM_cpy_to_checkpoint(pool);
#endif

    tid_array = (pthread_t *)malloc(sizeof(pthread_t) * thread_max);

    bptreeThreadInit(BPTREE_BLOCK);

    arg_t *arg = NULL;
    for (i = 0; i < thread_max-1; i++) {
        arg = (arg_t *)malloc(sizeof(int));
        arg->seed = i;
        arg->tid = i % 256 + 1;
        arg->loop = loop_times / thread_max;
        tid_array[i] = bptreeCreateThread(bpt, delete_random, arg);
    }
    arg = (arg_t *)malloc(sizeof(int));
    arg->seed = i;
    arg->tid = i % 256 + 1;
    arg->loop = loop_times / thread_max + loop_times % thread_max;
    tid_array[i] = bptreeCreateThread(bpt, delete_random, arg);

    clock_gettime(CLOCK_MONOTONIC_RAW, &stt);
    bptreeStartThread();

    for (i = 0; i < thread_max; i++) {
        bptreeWaitThread(tid_array[i], (void **)&arg);
        free(arg);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &edt);

    bptreeThreadDestroy();

    destroyAllocator();

    fprintf(stderr, "finish running threads\n");

    double time = edt.tv_nsec - stt.tv_nsec;
    time /= 1000000000;
    time += edt.tv_sec - stt.tv_sec;
    printf("%lf\n", time);

#ifdef NVHTM
    NVHTM_shutdown();
#endif

    // showTree(bpt, 0);

    return 0;
}
