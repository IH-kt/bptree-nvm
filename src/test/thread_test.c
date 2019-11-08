#include "fptree.h"
#include "thread_manager.h"
#include <stdlib.h>
#include <time.h>

#define SEED_INIT_VAL 0

int loop_times = 40;
int max_val = 1000;

void *insert_test(BPTree *bpt, void *arg) {
    KeyValuePair kv;
    unsigned int seed = SEED_INIT_VAL;
    kv.key = 1;
    kv.value = 1;
    for (int i = 1; i <= loop_times; i++) {
        kv.key = rand_r(&seed) % max_val + 1;
        printf("insert: target = %ld\n", kv.key);
        if (insert(bpt, kv)) {
            printf("insert: success\n");
        } else {
            printf("insert: failure\n");
        }
    }
    printf("insert: tree state -----------------------------\n");
    showTree(bpt);
    printf("------------------------------------------------\n");
    return NULL;
}

void *search_test(BPTree *bpt, void *arg) {
    Key key = 1;
    unsigned int seed = SEED_INIT_VAL;
    struct timespec wait_time;
    wait_time.tv_sec = 0;
    wait_time.tv_nsec = 10;
    SearchResult sr;
    for (int i = 1; i <= loop_times; i++) {
        key = rand() % max_val;
        printf("search: target = %ld\n", key);
        search(bpt, key, &sr);
        if (sr.index != -1) {
            printf("search: found\n");
        } else {
            printf("search: not found\n");
        }
	nanosleep(&wait_time, NULL);
    }
    printf("search: tree state -----------------------------\n");
    showTree(bpt);
    printf("------------------------------------------------\n");
    return NULL;
}

void *delete_test(BPTree *bpt, void *arg) {
    Key key;
    unsigned int seed = SEED_INIT_VAL;
    key = 1;
    for (int i = 1; i <= loop_times; i++) {
        key = rand_r(&seed) % max_val + 1;
        printf("delete: target = %ld\n", key);
        if (delete(bpt, key)) {
            printf("delete: success\n");
        } else {
            printf("delete: failure\n");
        }
    }
    printf("delete: tree state -----------------------------\n");
    showTree(bpt);
    printf("------------------------------------------------\n");
    return NULL;
}

int main(int argc, char *argv[]) {
    BPTree *bpt = newBPTree();
    KeyValuePair kv;
    if (argc > 1) {
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
	printf("loop_times = %d, max_val = %d\n", loop_times, max_val);
    } else {
        printf("default: loop_times = 40, max_val = 1000\n");
    }

    pthread_t tid[3];

    bptreeThreadInit(BPTREE_BLOCK);
    printf("init\n");

    tid[0] = bptreeCreateThread(bpt, insert_test, NULL);
    tid[1] = bptreeCreateThread(bpt, search_test, NULL);
    tid[2] = bptreeCreateThread(bpt, delete_test, NULL);
    printf("create\n");

    bptreeStartThread();
    printf("start\n");

    bptreeWaitThread(tid[0], NULL);
    bptreeWaitThread(tid[1], NULL);
    bptreeWaitThread(tid[2], NULL);
    printf("wait\n");

    bptreeThreadDestroy();
    printf("destroy\n");

    printf("finish running threads\n");
    showTree(bpt);

    return 0;
}
