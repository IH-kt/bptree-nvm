#include "fptree.h"
#include <stdlib.h>

#define TIMES 100

int main(int argc, char *argv[]) {
    BPTree *bpt = newBPTree();
    KeyValuePair kv;
    kv.key = 1;
    kv.value = 1;
    srand(1);
    for (int i = 1; i <= TIMES; i++) {
        kv.key = rand()%4000;
        // printf("insert %d\n", kv.key);
        insert(bpt, kv);
    }

    SearchResult sr;
    srand(1);
    for (int i = 1; i <= TIMES; i++) {
        kv.key = rand()%4000;
        search(bpt, kv.key, &sr);
        if (sr.index == -1) {
            printf("not found: %d\n", kv.key);
            break;
        }
        printf("%d => (%p, %d)\n", kv.key, sr.node, sr.index);
    }

    showTree(bpt);
    return 0;
}
