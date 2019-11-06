#include "fptree.h"
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    BPTree *bpt = newBPTree();
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
    kv.key = 1;
    kv.value = 1;
    srand((unsigned) time(NULL));
    for (int i = 1; i <= loop_times; i++) {
        kv.key = rand() % max_val;
        printf("insert %d\n", kv.key);
        if (insert(bpt, kv)) {
            printf("success\n");
        } else {
            printf("failure\n");
        }
    }
    for (int i = 1; i <= loop_times; i++) {
        kv.key = rand() % max_val;
        printf("delete %d\n", kv.key);
        if (delete(bpt, kv.key)) {
            printf("success\n");
        } else {
            printf("failure\n");
        }
    }

    showTree(bpt);

    return 0;
}
