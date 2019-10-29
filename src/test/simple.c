#include "fptree.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    BPTree *bpt = newBPTree();
    KeyValuePair kv;
    kv.key = 1;
    kv.value = 1;
    srand(1);
    for (int i = 1; i <= 100; i++) {
        kv.key = rand()%4000;
        insert(bpt, kv);
    }

    SearchResult sr;
    srand(1);
    for (int i = 1; i <= 100; i++) {
        kv.key = rand()%4000;
        search(bpt, kv.key, &sr);
        printf("%d => (%p, %d)\n", kv.key, sr.node, sr.index);
    }

    showTree(bpt);
    return 0;
}
