#include "fptree.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    BPTree *bpt = newBPTree();
    KeyValuePair kv;
    kv.key = 1;
    kv.value = 1;
    srand(1);

    kv.key = rand();
    printf("insert %ld\n", kv.key);
    insert(bpt, kv, 0);

    SearchResult sr;
    showTree(bpt, 0);
    search(bpt, kv.key, &sr, 0);
    if (sr.index == -1) {
        printf("not found: %ld\n", kv.key);
    }
    printf("search: %ld => (%p, %d)\n", kv.key, sr.node, sr.index);

    printf("delete %ld\n", kv.key);
    delete(bpt, kv.key, 0);

    showTree(bpt, 0);
    return 0;
}
