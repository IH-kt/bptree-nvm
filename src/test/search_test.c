#include "fptree.h"
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    BPTree *bpt;
    KeyValuePair kv;
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
#ifdef NVHTM
#else
    initAllocator(NULL, "data", sizeof(LeafNode) * loop_times / MAX_KEY * 2, 1);
#endif
    bpt = newBPTree();
    kv.key = 1;
    kv.value = 1;
    srand((unsigned) time(NULL));
    for (int i = 1; i <= loop_times; i++) {
        kv.key = rand() % max_val;
        printf("insert %ld\n", kv.key);
        if (insert(bpt, kv, 0)) {
            printf("success\n");
        } else {
            printf("failure\n");
        }
    }
    SearchResult sr;
    for (int i = 1; i <= loop_times; i++) {
        kv.key = rand() % max_val;
        printf("search %ld\n", kv.key);
        search(bpt, kv.key, &sr, 0);
        if (sr.index != -1) {
            printf("found\n");
        } else {
            printf("not found\n");
        }
    }

    showTree(bpt, 0);
    destroyAllocator();

    return 0;
}
