#include "allocator.c"

#define HEAP_SIZE 8 * 1024

struct Node {
    MyallocPAddr pnext;
    int y;
    struct Node *next;
};

struct Root {
    MyallocPAddr phead;
    int x;
    struct Node *head;
};

int main() {
    if (myalloc_init("/Users/c/lab/git/iiboshi-research/nvm/btrees-src/tempfile", HEAP_SIZE)) {
        printf("error\n");
        return 1;
    }
    printf("init\n");
    struct Root *root = (struct Root *)myalloc_root_allocate(sizeof(struct Root), sizeof(struct Node));
    printf("allocate:%p\n", root);
    root->x = 1192;
    printf("write one\n");
    root->head = (struct Node *)myalloc_node_allocate();
    root->phead = myalloc_get_persistent_addr(root->head);
    struct Node *current = root->head;

    printf("allocate:%p\n", current);

    int i = 0;
    for (i = 0; i < 50; i++) {
        current->y = i;
        current->next = (struct Node *)myalloc_node_allocate();
        current->pnext = myalloc_get_persistent_addr(current->next);
        printf("current->next:%p, pnext:%zu\n", current->next, current->pnext.offset);
        current->next->next = NULL;
        current->next->pnext = myalloc_get_persistent_addr(NULL);
        current = current->next;
    }
    current->y = i;
    printf("current->next:%p, pnext:%zu\n", current->next, current->pnext.offset);

    struct Node *prevfreetarget = root->head->next;
    struct Node *freetarget = prevfreetarget->next;
    prevfreetarget->next = freetarget->next;
    prevfreetarget->pnext = freetarget->pnext;
    myalloc_node_free(freetarget);

    myalloc_end();
    return 0;
}
