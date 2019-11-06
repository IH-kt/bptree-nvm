#include "allocator.c"

#define HEAP_SIZE 8 * 1024

typedef struct Node {
    MyallocPAddr pnext;
    int y;
    struct Node *next;
} Node;

typedef struct Root {
    MyallocPAddr phead;
    int x;
    struct Node *head;
} Root;

MyallocPAddr next(void *ptr) {
    return ((struct Node *)ptr)->pnext;
}

int main() {
    if (myalloc_recover("/Users/c/lab/git/iiboshi-research/nvm/btrees-src/tempfile", HEAP_SIZE, sizeof(struct Root), sizeof(struct Node), next)) {
        printf("error\n");
        return 1;
    }
    printf("init\n");
    struct Root *root = (struct Root *)myalloc_get_root();

    printf("root:%p, %d, nxt=%p\n", root, root->x, myalloc_get_raw_addr(root->phead));

    Node *current = myalloc_get_raw_addr(root->phead);
    Node *prev;
    while (current != NULL) {
        printf("current:%p, %d, next=%p, pnext=%zu\n", current, current->y, myalloc_get_raw_addr(current->pnext), current->pnext.offset);
        prev = current;
        current = myalloc_get_raw_addr(current->pnext);
        prev->next = current;
    }

    Node *new = myalloc_node_allocate();
    printf("newly allocated %p\n", new);
    printf("prev: %p\n", prev);
    new->y = 0x0;
    prev->next = new;
    prev->pnext = myalloc_get_persistent_addr(new);
    printf("prev->next:%p, prev->pnext:%zx\n", prev->next, prev->pnext.offset);

    new->next = NULL;
    new->pnext = myalloc_get_persistent_addr(NULL);

    myalloc_persist(prev, sizeof(Node));
    myalloc_persist(new, sizeof(Node));

    current = myalloc_get_raw_addr(root->phead);
    while (current != NULL) {
        printf("current:%p, %d, next=%p, pnext=%zu\n", current, current->y, myalloc_get_raw_addr(current->pnext), current->pnext.offset);
        current = myalloc_get_raw_addr(current->pnext);
    }

    myalloc_end();
    return 0;
}
