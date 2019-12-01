#include "tree.h"
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

void *insert_random(BPTree *bpt, void *arg) {
    arg_t *arg_cast = (arg_t *)arg;
#ifdef NVHTM
    NVHTM_thr_init();
#endif
    unsigned char tid = arg_cast->tid;
    KeyValuePair kv;
    unsigned int seed = arg_cast->seed;
    unsigned int loop = arg_cast->loop;
    kv.key = 1;
    kv.value = 1;
    for (int i = 1; i <= loop; i++) {
        kv.key = rand_r(&seed) % max_val + 1;
        // printf("inserting %ld\n", kv.key);
        if (!insert(bpt, kv, tid)) {
            // fprintf(stderr, "insert: failure\n");
        }
        // showTree(bpt);
    }
    show_result_thread(tid);
#ifdef NVHTM
    NVHTM_thr_exit();
#endif
    return arg;
}

void *search_random(BPTree *bpt, void *arg) {
    arg_t *arg_cast = (arg_t *)arg;
#ifdef NVHTM
    NVHTM_thr_init();
#endif
    unsigned char tid = arg_cast->tid;
    SearchResult sr;
    Key key = 1;
    unsigned int seed = arg_cast->seed;
    unsigned int loop = arg_cast->loop;
    for (int i = 1; i <= loop; i++) {
        key = rand_r(&seed) % max_val + 1;
        search(bpt, key, &sr, tid);
    }
    show_result_thread(tid);
#ifdef NVHTM
    NVHTM_thr_exit();
#endif
    return NULL;
}

char const *pmem_path = "data";
char const *log_path = "log";

int main(int argc, char *argv[]) {
    pthread_t *tid_array;
    arg_t **arg;
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
        log_path = argv[6];
        fprintf(stderr, "warm_up = %d, loop_times = %d, max_val = %d, thread_max = %d, pmem_path = %s, log_path = %s\n", warm_up, loop_times, max_val, thread_max, pmem_path, log_path);
    } else {
        fprintf(stderr, "default: warm_up = %d, loop_times = %d, max_val = %d, thread_max = %d, pmem_path = %s, log_path = %s\n", warm_up, loop_times, max_val, thread_max, pmem_path, log_path);
    }

#ifdef BPTREE
    size_t allocation_size = sizeof(LeafNode) * (warm_up / (MAX_PAIR/2) + 2 + thread_max) + sizeof(AllocatorHeader);
#else
    size_t allocation_size = sizeof(PersistentLeafNode) * (warm_up / (MAX_PAIR/2) + 2 + thread_max) + sizeof(AllocatorHeader);
#endif
    fprintf(stderr, "allocating %lu byte\n", allocation_size);
#ifdef NVHTM
    set_log_file_name(log_path);
    NVHTM_init(thread_max + 2);
    void *pool = NH_alloc(pmem_path, allocation_size);
    NVHTM_clear();
    NVHTM_cpy_to_checkpoint(pool);
    initAllocator(pool, pmem_path, allocation_size, thread_max + 1);
#else
    initAllocator(NULL, pmem_path, allocation_size, thread_max + 1);
#endif
    bpt = newBPTree();

    tid_array = (pthread_t *)malloc(sizeof(pthread_t) * (thread_max + 1));
    arg = (arg_t **)malloc(sizeof(arg_t *) * (thread_max + 1));

    bptreeThreadInit(BPTREE_NONBLOCK);

    kv.key = 1;
    kv.value = 1;
    unsigned int seed = thread_max;
    arg[thread_max] = (arg_t *)malloc(sizeof(arg_t));
    arg[thread_max]->seed = thread_max;
    arg[thread_max]->tid = thread_max;
    arg[thread_max]->loop = warm_up;
    tid_array[thread_max] = bptreeCreateThread(bpt, insert_random, arg[thread_max]);
    bptreeWaitThread(tid_array[thread_max], NULL);
    free(arg[thread_max]);

    bptreeThreadInit(BPTREE_BLOCK);

    for (i = 0; i < thread_max-1; i++) {
        arg[i] = (arg_t *)malloc(sizeof(int));
        arg[i]->seed = i;
        arg[i]->tid = i % 256 + 1;
        arg[i]->loop = loop_times / thread_max;
        tid_array[i] = bptreeCreateThread(bpt, search_random, arg[i]);
    }
    arg[i] = (arg_t *)malloc(sizeof(int));
    arg[i]->seed = i;
    arg[i]->tid = i % 256 + 1;
    arg[i]->loop = loop_times / thread_max + loop_times % thread_max;
    tid_array[i] = bptreeCreateThread(bpt, search_random, arg[i]);

    clock_gettime(CLOCK_MONOTONIC_RAW, &stt);
    bptreeStartThread();

    for (i = 0; i < thread_max; i++) {
        bptreeWaitThread(tid_array[i], NULL);
        free(NULL);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &edt);

    fprintf(stderr, "finish running threads\n");

    double time = edt.tv_nsec - stt.tv_nsec;
    time /= 1000000000;
    time += edt.tv_sec - stt.tv_sec;
    printf("%lf\n", time);

    // showTree(bpt, 0);

    destroyBPTree(bpt, 1);

    bptreeThreadDestroy();

    destroyAllocator();

#ifdef NVHTM
    NVHTM_shutdown();
#endif

    return 0;
}
