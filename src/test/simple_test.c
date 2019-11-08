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
    insert(bpt, kv);

    SearchResult sr;
    showTree(bpt);
    search(bpt, kv.key, &sr);
    if (sr.index == -1) {
        printf("not found: %d\n", kv.key);
    }
    printf("search: %ld => (%p, %d)\n", kv.key, sr.node, sr.index);

    printf("delete %ld\n", kv.key);
    delete(bpt, kv.key);

    showTree(bpt);
    return 0;
}
