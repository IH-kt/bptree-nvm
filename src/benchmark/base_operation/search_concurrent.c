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

void *search_random(BPTree *bpt, void *arg) {
    arg_t *arg_cast = (arg_t *)arg;
    unsigned char tid = arg_cast->tid;
    SearchResult sr;
    Key key = 1;
    unsigned int seed = arg_cast->seed;
    unsigned int loop = arg_cast->loop;
    for (int i = 1; i <= loop; i++) {
        key = rand_r(&seed) % max_val + 1;
        search(bpt, key, &sr, tid);
    }
#ifdef TIME_PART
    showTime(tid);
#endif
    return NULL;
}

void *main_thread_func(void *arg_p) {
    pthread_t *tid_array;
    struct timespec stt, edt;
    int i;
    BPTree *bpt;
    KeyValuePair kv;
    NVHTM_thr_init();
    /*
    bpt = newBPTree();

    kv.key = 1;
    kv.value = 1;
    unsigned int seed = thread_max;
    for (i = 0; i < warm_up; i++) {
        printf("i = %d\n", i);
        kv.key = rand_r(&seed) % max_val + 1;
        insert(bpt, kv, thread_max);
    }

    tid_array = (pthread_t *)malloc(sizeof(pthread_t) * thread_max);

    bptreeThreadInit(BPTREE_BLOCK);

    arg_t *arg = NULL;
    for (i = 0; i < thread_max-1; i++) {
        arg = (arg_t *)malloc(sizeof(int));
        arg->seed = i;
        arg->tid = i % 256 + 1;
        arg->loop = loop_times / thread_max;
        tid_array[i] = bptreeCreateThread(bpt, search_random, arg);
    }
    arg = (arg_t *)malloc(sizeof(int));
    arg->seed = i;
    arg->tid = i % 256 + 1;
    arg->loop = loop_times / thread_max + loop_times % thread_max;
    tid_array[i] = bptreeCreateThread(bpt, search_random, arg);

    clock_gettime(CLOCK_MONOTONIC_RAW, &stt);
    bptreeStartThread();

    for (i = 0; i < thread_max; i++) {
        bptreeWaitThread(tid_array[i], (void **)&arg);
        free(arg);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &edt);

    fprintf(stderr, "finish running threads\n");

    double time = edt.tv_nsec - stt.tv_nsec;
    time /= 1000000000;
    time += edt.tv_sec - stt.tv_sec;
    printf("%lf\n", time);

    bptreeThreadDestroy();
    */
    NVHTM_thr_exit();
}

char const *pmem_path = "data";

int main(int argc, char *argv[]) {
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
        fprintf(stderr, "default: warm_up = %d, loop_times = %d, max_val = %d, thread_max = %d, pmem_path = %s\n", warm_up, loop_times, max_val, thread_max, pmem_path);
    }

    size_t allocation_size = sizeof(PersistentLeafNode) * loop_times / (MAX_PAIR / 3);
#ifdef NVHTM
    NVHTM_init(thread_max+1);
    void *pool = NH_alloc(allocation_size);
    printf("write pool\n");
    *(int *)pool = 0;
    printf("read pool: %d\n", *(int *)pool);
    printf("mapped:%p, %lu\n", pool, allocation_size);
    // initAllocator(pool, pmem_path, allocation_size, thread_max+1);
    NVHTM_clear();
    NVHTM_cpy_to_checkpoint(pool);
#else
    initAllocator(NULL, pmem_path, allocation_size, thread_max+1);
#endif

    pthread_t main_id;
    pthread_create(&main_id, NULL, main_thread_func, NULL);
    pthread_join(main_id, NULL);

    // destroyAllocator();


#ifdef NVHTM
    NVHTM_shutdown();
#endif
    // showTree(bpt, 0);

    return 0;
}
