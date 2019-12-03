#include "tree.h"
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    BPTree *bpt;
    KeyValuePair kv;
    SearchResult sr;
    int loop_times = 40;
    int max_val = 1000;
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
    } else {
        printf("default: loop_times = 40, max_val = 1000\n");
    }

    size_t allocation_size = sizeof(PersistentLeafNode) * ((loop_times * 2 / (MAX_PAIR / 2)) + 3) + sizeof(AllocatorHeader);
    set_log_file_name("./log");
    NVHTM_init(3);
    void *pool = NH_alloc("./data", allocation_size);
    NVHTM_clear();
    NVHTM_cpy_to_checkpoint(pool);
    initAllocator(pool, "data", allocation_size, 1);

    bpt = newBPTree();
    kv.key = 1;
    kv.value = 1;
    NVHTM_thr_init();
    for (int i = 1; i <= loop_times; i++) {
        kv.key = i;
        printf("insert %ld\n", kv.key);
        if (insert(bpt, kv, 1)) {
            printf("success\n");
        } else {
            printf("failure\n");
        }
    }
    for (int i = 1; i <= loop_times; i++) {
        kv.key = i;
        printf("search %ld\n", kv.key);
        search(bpt, kv.key, &sr, 1);
        if (sr.index != -1) {
            printf("found\n");
        } else {
            printf("not found\n");
            assert(0);
        }
    }
    srand(loop_times);
    for (int i = 1; i <= loop_times; i++) {
        kv.key = rand() % max_val;
        printf("insert %ld\n", kv.key);
        if (insert(bpt, kv, 1)) {
            printf("success\n");
        } else {
            printf("failure\n");
        }
    }
    srand(loop_times);
    for (int i = 1; i <= loop_times; i++) {
        kv.key = rand() % max_val;
        printf("search %ld\n", kv.key);
        search(bpt, kv.key, &sr, 1);
        if (sr.index != -1) {
            printf("found\n");
        } else {
            assert(0);
            printf("not found\n");
        }
    }
    NVHTM_thr_exit();

    showTree(bpt, 1);
    destroyAllocator();
    NVHTM_shutdown();

    return 0;
}
