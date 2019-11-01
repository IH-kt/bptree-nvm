#include "fptree.h"
#include "allocator.h"

#ifndef NPERSIST
void persist(void *target, size_t size) {
    int i;
    for (i = 0; i < (size-1)/64 + 1; i++) {
#  ifdef CLWB
        _mm_clwb(target + i * 64);
#  else
        _mm_clflush(target + i * 64);
#  endif
    }
    _mm_sfence();
}
#else
void persist(void *target, size_t size) { /* EMPTY */ }
#endif

/* utils */
unsigned char hash(Key key) {
    return key % 256;
}

char popcntcharsize(char bits) {
    bits = (bits & 0x55) + (bits>>1 & 0x55);
    bits = (bits & 0x33) + (bits>>2 & 0x33);
    bits = (bits & 0x0f) + (bits>>4 & 0x0f);
    return bits;
}

/* initializer */
void initKeyValuePair(KeyValuePair *pair) {
    pair->key = UNUSED_KEY;
    pair->value = INITIAL_VALUE;
}

void initLeafNode(LeafNode *node) {
    int i;
    PersistentLeafNode *new_pleaf = (PersistentLeafNode *)pmem_allocate(sizeof(PersistentLeafNode));
    for (i = 0; i < BITMAP_SIZE; i++) {
        new_pleaf->header.bitmap[i] = 0;
    }
    new_pleaf->header.pnext = P_NULL;
    for (i = 0; i < MAX_PAIR; i++) {
        new_pleaf->header.fingerprints[i] = 0;
        initKeyValuePair(&new_pleaf->kv[i]);
    }
    new_pleaf->lock = 0;
    node->pleaf = new_pleaf;
    node->next = NULL;
    node->prev = NULL;
    node->key_length = 0;
}

void initInternalNode(InternalNode *node) {
    int i;
    node->key_length = 0;
    node->children_type = LEAF;
    node->parent = NULL;
    for (i = 0; i < MAX_KEY; i++) {
        node->keys[i] = UNUSED_KEY;
    }
    for (i = 0; i < MAX_DEG; i++) {
        node->children[i] = NULL;
    }
}

void initBPTree(BPTree *tree, LeafNode *leafHead, InternalNode *rootNode, ppointer *pmem_head) {
    tree->pmem_head = pmem_head;
    *pmem_head = getPersistentAddr(leafHead->pleaf);
    tree->head = leafHead;
    tree->root = rootNode;
    rootNode->children_type = LEAF;
    rootNode->children[0] = leafHead;
}

void initSearchResult(SearchResult *sr) {
    sr->node = NULL;
    sr->index = -1;
}

LeafNode *newLeafNode() {
    LeafNode *new = (LeafNode *)vmem_allocate(sizeof(LeafNode));
    initLeafNode(new);
    return new;
}
void destroyLeafNode(LeafNode *node) {
    pmem_free(node);
}

InternalNode *newInternalNode() {
    InternalNode *new = (InternalNode *)vmem_allocate(sizeof(InternalNode));
    initInternalNode(new);
    return new;
}
void destroyInternalNode(InternalNode *node) {
    vmem_free(node);
}

BPTree *newBPTree() {
    ppointer *pmem_head = root_allocate(sizeof(ppointer), sizeof(PersistentLeafNode));
    BPTree *new = (BPTree *)vmem_allocate(sizeof(BPTree));
    InternalNode *rootNode = newInternalNode();
    LeafNode *leafHead = newLeafNode();
    initBPTree(new, leafHead, rootNode, pmem_head);
    return new;
}
void destroyBPTree(BPTree *tree) {
    destroyLeafNode(tree->head);
    destroyInternalNode(tree->root);
    *tree->pmem_head = P_NULL;
    root_free(tree->pmem_head);
    vmem_free(tree);
}

// int getLeafNodeLength(LeafNode *ln) {
//     int key_length = 0, i;
//     for (i = 0; i < BITMAP_SIZE-1; i++) {
//         key_length += popcntcharsize(ln->header.bitmap[i]);
//     }
//     key_length += popcntcharsize(ln->header.bitmap[i] & (0xff >> (8-MAX_PAIR%8)));
// #ifdef DEBUG
//     printf("getLeafNodeLength:%p's key_length = %d\n", ln, key_length);
// #endif
//     return key_length;
// }

int searchInLeaf(LeafNode *node, Key key) {
    int i;
    unsigned char key_fp = hash(key);
    if (node == NULL) {
        return -1;
    }
    for (i = 0; i < MAX_PAIR; i++) {
        if (GET_BIT(node->pleaf->header.bitmap, i) != 0 &&
                node->pleaf->header.fingerprints[i] == key_fp &&
                node->pleaf->kv[i].key == key) {
            return i;
        }
    }
    return -1;
}

LeafNode *findLeaf(InternalNode *current, Key targetkey, InternalNode **parent) {
    int i;
    for (i = 0; i < current->key_length; i++) {
        if (targetkey <= current->keys[i]) {
            break;
        }
    }
    int key_index = i;
    if (current->children_type == LEAF) {
        if (parent != NULL) {
            *parent = current;
        }
        return (LeafNode *)current->children[key_index];
    } else {
        current = current->children[key_index];
        return findLeaf(current, targetkey, parent);
    }
}

/* search for targetkey in btree.
 * if it was not found, returns -1 index.
 * otherwise, returns nodelist and index of target pair.
 * destroySearchResult should be called after use.
 */
void search(BPTree *bpt, Key target_key, SearchResult *sr) {
    int i;
    initSearchResult(sr);

#ifdef DEBUG
    printf("search: key = %d\n", targetkey);
#endif

    if (bpt == NULL) {
#ifdef DEBUG
        printf("search: empty\n");
#endif
        return;
    } else {
        sr->node = findLeaf(bpt->root, target_key, NULL);
        sr->index = searchInLeaf(sr->node, target_key);
    }
}
 
int findFirstAvailableSlot(LeafNode *target) {
    int i;
    for (i = 0; i < MAX_PAIR; i++) {
        if (GET_BIT(target->pleaf->header.bitmap, i) == 0) {
            break;
        }
    }
    return i;
}

int compareKeyPositionPair(const void *a, const void *b) {
    return ((KeyPositionPair *)a)->key - ((KeyPositionPair *)b)->key;
}

void findSplitKey(LeafNode *target, Key *split_key, char *bitmap) {
    int i;
    KeyPositionPair pairs[MAX_PAIR];

    // キーは全て有効（満杯の葉が入ってくるので）
    for (i = 0; i < MAX_PAIR; i++) {
        pairs[i].key = target->pleaf->kv[i].key;
        pairs[i].position = i;
    }

    for (i = 0; i < BITMAP_SIZE; i++) {
        bitmap[i] = 0;
    }

    qsort(pairs, MAX_PAIR, sizeof(KeyPositionPair), compareKeyPositionPair);
#ifdef DEBUG
    for (i = 0; i < MAX_PAIR; i++) {
        printf("sorted[%d] = %d\n", i, pairs[i].key);
    }
#endif

    *split_key = pairs[MAX_PAIR/2-1].key; // this becomes parent key

    for (i = MAX_PAIR/2; i < MAX_PAIR; i++) {
        SET_BIT(bitmap, pairs[i].position);
    }
#ifdef DEBUG
    printf("leaf splitted at %d\n", *split_key);
    for (i = 0; i < BITMAP_SIZE; i++) {
        printf("bitmap[%d] = %x\n", i, bitmap[i]);
    }
#endif
}

Key splitLeaf(LeafNode *target, KeyValuePair newkv) {
    LeafNode *new_leafnode = getTransientAddr(newLeafNode());
    int i;
    Key split_key;
    char bitmap[BITMAP_SIZE];

    for (i = 0; i < MAX_PAIR; i++) {
        new_leafnode->pleaf->kv[i] = target->pleaf->kv[i];
        new_leafnode->pleaf->header.fingerprints[i] = target->pleaf->header.fingerprints[i];
    }
    new_leafnode->pleaf->header.pnext = target->pleaf->header.pnext;
    new_leafnode->pleaf->lock = target->pleaf->lock;
    persist(new_leafnode->pleaf, sizeof(PersistentLeafNode));

    findSplitKey(new_leafnode, &split_key, bitmap);

    for (i = 0; i < BITMAP_SIZE; i++) {
        new_leafnode->pleaf->header.bitmap[i] = bitmap[i];
    }
    persist(new_leafnode->pleaf->header.bitmap, BITMAP_SIZE * sizeof(bitmap[0]));

    for (i = 0; i < BITMAP_SIZE; i++) {
        target->pleaf->header.bitmap[i] = ~bitmap[i];
    }
    persist(target->pleaf->header.bitmap, BITMAP_SIZE * sizeof(bitmap[0]));

    target->pleaf->header.pnext = getPersistentAddr(new_leafnode->pleaf);

    persist(target->pleaf->header.pnext, sizeof(LeafNode *));

    new_leafnode->next = target->next;
    new_leafnode->prev = target;
    if (target->next != NULL) {
        target->next->prev = new_leafnode;
    }
    target->next = new_leafnode;

    target->key_length = MAX_PAIR/2;
    new_leafnode->key_length = MAX_PAIR - MAX_PAIR/2;

    // insert new node
    if (newkv.key <= split_key) {
        insertNonfullLeaf(target, newkv);
    } else {
        insertNonfullLeaf(new_leafnode, newkv);
    }

    return split_key;
}

Key splitInternal(InternalNode *target, InternalNode **splitted_node, void *new_node, Key new_key) {
    int i;
    int split_position = -1;
    if (new_key <= target->keys[MAX_KEY/2]) {
        split_position = MAX_KEY/2;
    } else {
        split_position = MAX_KEY/2 + 1;
    }
    InternalNode *new_splitted_node = newInternalNode();
    new_splitted_node->key_length = MAX_KEY - (split_position+1);
    int newnode_length = new_splitted_node->key_length;

    for (i = 0; i < newnode_length; i++) {
        new_splitted_node->keys[i] = target->keys[i+split_position+1];
        new_splitted_node->children[i] = target->children[i+split_position+1];
        target->keys[i+split_position+1] = UNUSED_KEY;
        target->children[i+split_position+1] = NULL;
    }
    new_splitted_node->children[i] = target->children[i+split_position+1];
    target->children[i+split_position+1] = NULL;

    new_splitted_node->parent = target->parent;
    new_splitted_node->children_type = target->children_type;

    Key split_key = target->keys[split_position];
    target->keys[split_position] = UNUSED_KEY;
    target->key_length -= newnode_length + 1;

    *splitted_node = new_splitted_node;

    if (new_key <= split_key) {
        insertNonfullInternal(target, new_key, new_node);
    } else {
        insertNonfullInternal(new_splitted_node, new_key, new_node);
    }

    return split_key;
}

// assumption: child is right hand of key
void insertNonfullInternal(InternalNode *target, Key key, void *child) {
    int i;
    for (i = target->key_length; 0 < i && key < target->keys[i-1]; i--) {
        target->keys[i] = target->keys[i-1];
        target->children[i+1] = target->children[i];
    }
    target->children[i+1] = child;
    target->keys[i] = key;
    target->key_length++;
}

void insertNonfullLeaf(LeafNode *node, KeyValuePair kv) {
    int slot = findFirstAvailableSlot(node);
    node->pleaf->kv[slot] = kv;
    node->pleaf->header.fingerprints[slot] = hash(kv.key);
    persist(&node->pleaf->kv[slot], sizeof(KeyValuePair));
    persist(&node->pleaf->header.fingerprints[slot], sizeof(char));
    SET_BIT(node->pleaf->header.bitmap, slot);
    persist(&node->pleaf->header.bitmap[slot/8], sizeof(char));
    node->key_length++;
}

void insert(BPTree *bpt, KeyValuePair kv) {
    if (bpt == NULL) {
        return;
    }

    InternalNode *parent;
    LeafNode *target_leaf = findLeaf(bpt->root, kv.key, &parent);
    // lock(target_leaf);

    if (target_leaf->key_length < MAX_PAIR) {
        insertNonfullLeaf(target_leaf, kv);
    } else {
        Key split_key = splitLeaf(target_leaf, kv);
        LeafNode *new_leaf = target_leaf->next;
        // start tx;
        insertParent(bpt, parent, split_key, new_leaf, target_leaf);
        // end tx;
        // unlock(new_leaf);
    }
    // unlock(target_leaf);
}

int findNodeInInternalNode(InternalNode *parent, void *target) {
    int i;
    for (i = 0; i <= parent->key_length; i++) {
        if (parent->children[i] == target) {
            return 1;
        }
    }
    return 0;
}

void insertParent(BPTree *bpt, InternalNode *parent, Key new_key, LeafNode *new_leaf, LeafNode *target_leaf) {
    if (parent->key_length < MAX_KEY && findNodeInInternalNode(parent, target_leaf) == 1) {
        insertNonfullInternal(parent, new_key, new_leaf);
    } else {
        Key split_key;
        InternalNode *split_node;
        int splitted = insertUpward(bpt->root, new_key, new_leaf, &split_key, &split_node);
        if (splitted) {
            // need to update root
            InternalNode *new_root = newInternalNode();
            new_root->children[0] = bpt->root;
            insertNonfullInternal(new_root, split_key, split_node);
            bpt->root->parent = new_root;
            split_node->parent = new_root;
            new_root->children_type = INTERNAL;
            bpt->root = new_root;
        }
    }
}

// new_node is right hand of new_key
int insertUpward(InternalNode *current, Key new_key, LeafNode *new_node, Key *split_key, InternalNode **split_node) {
    int i;
    if (current->children_type == LEAF) {
        if (current->key_length < MAX_KEY) {
            insertNonfullInternal(current, new_key, new_node);
            return 0;
        } else {
            // this need split
            *split_key = splitInternal(current, split_node, new_node, new_key);
            return 1;
        }
    } else {
        for (i = 0; i < current->key_length; i++) {
            if (new_key <= current->keys[i]) {
                break;
            }
        }
        int splitted = insertUpward(current->children[i], new_key, new_node, split_key, split_node);
        if (splitted) {
            if (current->key_length < MAX_KEY) {
                insertNonfullInternal(current, *split_key, *split_node);
                return 0;
            } else {
                InternalNode *new_split_node;
                Key new_split_key = splitInternal(current, &new_split_node, *split_node, *split_key);
                // Key new_split_key = splitInternal(current, &new_split_node);
                // if (*split_key <= new_split_key) {
                //     insertNonfullInternal(current, *split_key, *split_node);
                // } else {
                //     insertNonfullInternal(new_split_node, *split_key, *split_node);
                // }
                *split_key = new_split_key;
                *split_node = new_split_node;
                return 1;
            }
        }
        return 0;
    }
}

// void deleteLeaf(BPTree *bpt, LeafNode *current) {
//     if (current->prev == NULL) {
//         bpt->head = current->next;
//         *bpt->pmem_head = current->pleaf->header.pnext;
//         persist(bpt->pmem_head, sizeof(ppointer));
//     } else {
//         current->prev->pleaf->header.pnext = current->pleaf->header.next;
//         current->prev->next = current->next;
//     }
//     if (current->next != NULL) {
//         current->next->prev = current->prev;
//     }
//     destroyLeafNode(current);
// }
// 
// InternalNode *collapseRoot(InternalNode *oldroot) {
//     if (oldroot->children_type == LEAF) {
//         return NULL;
//     } else {
//         return (InternalNode *)oldroot->children[0];
//     }
// }
// 
// int delete(BPTree *bpt, Key target_key) {
//     SearchResult sr;
//     // start tx
//     LeafNode *target_leaf = findLeaf(bpt->root, target_key, NULL);
//     if (target_leaf == NULL) {
//         return 0;
//     }
//     if (target_leaf->key_length == 1) {
//         // lock target_leaf
//         // lock target_leaf->prev
//         rebalanceUpward();
//     } else {
//         // lock target_leaf
//         int keypos = searchInLeaf(target_leaf, target_key);
//         if (keypos == -1) {
// #ifdef DEBUG
//             printf("delete:the key doesn't exist. abort.\n");
// #endif
//             return 0;
//         }
//         CLR_BIT(target_leaf->pleaf->head.bitmap, keypos);
//         persist(target_leaf->pleaf->head.bitmap, BITMAP_SIZE);
//     }
//     // end tx
// 
//     InternalNode *new_root = collapseRoot();
//     if (new_root == NULL) {
//         return 0;
//     } else {
//         bpt->root = new_root;
//     }
// }

/* debug function */
void showLeafNode(LeafNode *node, int depth) {
    int i, j;
    for (j = 0; j < depth; j++)
        printf("\t");
    printf("leaf:%p (persistent -> %p)\n", node, node->pleaf);
    for (j = 0; j < depth; j++)
        printf("\t");
    printf("\tnext = %p, prev = %p, pnext = %p\n", node->next, node->prev, getTransientAddr(node->pleaf->header.pnext));
    for (j = 0; j < depth; j++)
        printf("\t");
    printf("\tlength = %d\n", node->key_length);
    for (i = 0; i < MAX_PAIR; i++) {
        for (j = 0; j < depth; j++)
            printf("\t");
        printf("\tbitmap[%d]:%d\n", i, GET_BIT(node->pleaf->header.bitmap, i));
    }
    for (i = 0; i < MAX_PAIR; i++) {
        if ((GET_BIT(node->pleaf->header.bitmap, i)) == 0)
            continue;
        for (j = 0; j < depth; j++)
            printf("\t");
        printf("\tkv[%d]:fgp = %d, key = %ld, val = %d\n", i, node->pleaf->header.fingerprints[i], node->pleaf->kv[i].key, node->pleaf->kv[i].value);
    }
}

void showInternalNode(InternalNode *node, int depth) {
    int i, j;
    for (j = 0; j < depth; j++)
        printf("\t");
    printf("internal:%p\n", node);
    for (j = 0; j < depth; j++)
        printf("\t");
    printf("\tlength = %d\n", node->key_length);
    for (i = 0; i < node->key_length; i++) {
        for (j = 0; j < depth; j++)
            printf("\t");
        printf("\tkey[%d] = %ld\n", i, node->keys[i]);
    }
    for (i = 0; i < node->key_length+1; i++) {
        for (j = 0; j < depth; j++)
            printf("\t");
        printf("\tchildren[%d]:\n", i);
        if (node->children_type == LEAF) {
            showLeafNode((LeafNode *)node->children[i], depth+1);
        } else {
            showInternalNode((InternalNode *)node->children[i], depth+1);
        }
    }
}

void showTree(BPTree *bpt) {
    printf("leaf head:%p\n", bpt->head);
    showInternalNode((InternalNode *)bpt->root, 0);
}
