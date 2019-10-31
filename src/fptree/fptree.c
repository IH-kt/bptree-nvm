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
    if (current->children_type == LEAF) {
        if (parent != NULL) {
            *parent = current;
        }
        return (LeafNode *)current->children[i];
    } else {
        current = current->children[i];
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

Key splitLeaf(LeafNode *target) {
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

    return split_key;
}

Key splitInternal(InternalNode *target, InternalNode **new_node) {
    int i;
    InternalNode *new_internalnode = newInternalNode();
    new_internalnode->key_length = MAX_KEY/2;

    for (i = 0; i < MAX_KEY/2; i++) {
        new_internalnode->keys[i] = target->keys[i+(MAX_KEY)/2];
        new_internalnode->children[i] = target->children[i+(MAX_KEY)/2];
        target->keys[i+(MAX_KEY)/2] = UNUSED_KEY;
        target->children[i+(MAX_KEY)/2] = NULL;
    }
    new_internalnode->children[i] = target->children[i+(MAX_KEY)/2];
    target->children[i+(MAX_KEY)/2] = NULL;

    new_internalnode->parent = target->parent;
    new_internalnode->children_type = target->children_type;

    Key split_key = target->keys[MAX_KEY/2-1];
    target->keys[MAX_KEY/2-1] = UNUSED_KEY;
    target->key_length -= MAX_KEY/2 + 1;

    *new_node = new_internalnode;
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
        Key split_key = splitLeaf(target_leaf);
        LeafNode *new_leaf = target_leaf->next;
        if (kv.key <= split_key) {
            insertNonfullLeaf(target_leaf, kv);
        } else {
            insertNonfullLeaf(new_leaf, kv);
        }
        // start tx;
        updateParent(bpt, parent, split_key, new_leaf);
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

void updateParent(BPTree *bpt, InternalNode *parent, Key new_key, LeafNode *new_leaf) {
    if (parent->key_length < MAX_KEY) {
        if (findNodeInInternalNode(parent, new_leaf) == 1) {
            insertNonfullInternal(parent, new_key, new_leaf);
            return;
        }
    }

    Key split_key;
    InternalNode *split_node;
    int splitted = updateUpward(bpt->root, new_key, new_leaf, &split_key, &split_node);
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

// new_node is right hand of new_key
int updateUpward(InternalNode *current, Key new_key, LeafNode *new_node, Key *split_key, InternalNode **split_node) {
    int i;
    if (current->children_type == LEAF) {
        if (current->key_length < MAX_KEY) {
            insertNonfullInternal(current, new_key, new_node);
            return 0;
        } else {
            // this need split
            *split_key = splitInternal(current, split_node);
            if (new_key <= *split_key) {
                insertNonfullInternal(current, new_key, new_node);
            } else {
                insertNonfullInternal(*split_node, new_key, new_node);
            }
            return 1;
        }
    } else {
        for (i = 0; i < current->key_length; i++) {
            if (new_key <= current->keys[i]) {
                break;
            }
        }
        int splitted = updateUpward(current->children[i], new_key, new_node, split_key, split_node);
        if (splitted) {
            if (current->key_length < MAX_KEY) {
                insertNonfullInternal(current, *split_key, *split_node);
                return 0;
            } else {
                InternalNode *new_split_node;
                Key new_split_key = splitInternal(current, &new_split_node);
                if (*split_key <= new_split_key) {
                    insertNonfullInternal(current, *split_key, *split_node);
                } else {
                    insertNonfullInternal(new_split_node, *split_key, *split_node);
                }
                *split_key = new_split_key;
                *split_node = new_split_node;
                return 1;
            }
        }
        return 0;
    }
}

// void deleteLeaf(BPTree *bpt, LeafNode *current, LeafNode *prev) {
//     DeleteLog *log = &bpt->deletelog.deletelog;
//     log->pcurrentleaf = getPersistentAddr(current);
//     persist(current, sizeof(LeafNode *));
//     if (current == bpt->head) {
//         bpt->head = getTransientAddr(current->header.next);
//         bpt->phead = current->header.next;
//         persist(&bpt->phead, sizeof(PAddr));
//     } else {
//         log->pprevleaf = getPersistentAddr(prev);
//         persist(&log->pprevleaf, sizeof(PAddr));
//         prev->header.next = current->header.next;
//         persist(&prev->header.next, sizeof(PAddr));
//     }
//     PFree(current);
// #ifdef DEBUG
//     showDeleteLog(log);
// #endif
//     resetMicroLog(&bpt->deletelog);
// }
// 
// void *collapseRoot(void *oldroot) {
//     void *newroot;
//     if (isPMEM(oldroot)) {
//         newroot = NULL;
//         PFree(oldroot);
//     } else {
//         newroot = ((InternalNode *)oldroot)->children[0];
//         DFree(oldroot);
//     }
//     return newroot;
// }
// 
// InternalNode *shift(InternalNode *currentnode, InternalNode *neighbornode, InternalNode *anchornode, char direction, void **balancenode) {
//     int i, shiftsize, currentnodeindex;
// #ifdef DEBUG
//     printf("shift:shifting\n");
// #endif
//     for (i = 0; i < anchornode->key_length; i++) {
//         if (currentnode->keys[0] < anchornode->keys[i]) {
//             break;
//         }
//     }
//     currentnodeindex = i;
//     shiftsize = (neighbornode->key_length - currentnode->key_length)/2;
//     if (direction == LEFT) {
//         for (i = currentnode->key_length-1; 0 <= i; i--) {
//             currentnode->keys[i+shiftsize] = currentnode->keys[i];
//         }
//         for (i = currentnode->key_length; 0 < i; i--) {
//             currentnode->children[i+shiftsize] = currentnode->children[i];
//         }
// 
//         currentnode->keys[shiftsize-1] = anchornode->keys[currentnodeindex-1];
//         for (i = 0; i < shiftsize-1; i++) {
//             currentnode->keys[shiftsize-2-i] = neighbornode->keys[neighbornode->key_length-1-i];
//             currentnode->children[shiftsize-1-i] = neighbornode->children[neighbornode->key_length-i];
//             neighbornode->keys[neighbornode->key_length-1-i] = UNUSED_KEY;
//             neighbornode->children[neighbornode->key_length-i] = NULL;
//         }
//         currentnode->children[0] = neighbornode->children[neighbornode->key_length-shiftsize+1];
//         neighbornode->children[neighbornode->key_length-shiftsize+1] = NULL;
// 
//         anchornode->keys[currentnodeindex-1] = neighbornode->keys[neighbornode->key_length-shiftsize];
//         neighbornode->keys[neighbornode->key_length-shiftsize] = UNUSED_KEY;
//     } else {
//         currentnode->keys[currentnode->key_length] = anchornode->keys[currentnodeindex];
//         for (i = 0; i < shiftsize-1; i++) {
//             currentnode->keys[currentnode->key_length+1+i] = neighbornode->keys[i];
//             currentnode->children[currentnode->key_length+1+i] = neighbornode->children[i];
//         }
//         currentnode->children[currentnode->key_length+shiftsize] = neighbornode->children[shiftsize-1];
//         anchornode->keys[currentnodeindex] = neighbornode->keys[shiftsize-1];
// 
//         for (i = 0; i + shiftsize < neighbornode->key_length; i++) {
//             neighbornode->keys[i] = neighbornode->keys[i + shiftsize];
//             neighbornode->children[i] = neighbornode->children[i + shiftsize];
//             neighbornode->keys[i + shiftsize] = UNUSED_KEY;
//             neighbornode->children[i + shiftsize] = NULL;
//         }
//         neighbornode->children[i] = neighbornode->children[i + shiftsize];
//         neighbornode->children[i + shiftsize] = NULL;
//     }
// 
//     neighbornode->key_length -= shiftsize;
//     currentnode->key_length += shiftsize;
// 
//     *balancenode = NULL;
// 
//     return NULL;
// }
// 
// InternalNode *merge(InternalNode *currentnode, InternalNode *neighbornode, InternalNode *anchornode, char direction) {
//     int i, currentnodeindex;
//     InternalNode *result;
// #ifdef DEBUG
//     printf("merge:current=%p, neighbor=%p, anchor=%p\n", currentnode, neighbornode, anchornode);
//     showInternalNode(anchornode, 0);
// #endif
//     for (i = 0; i < anchornode->key_length; i++) {
//         if (currentnode->keys[0] < anchornode->keys[i]) {
//             break;
//         }
//     }
//     if (i != 0) {
//         currentnodeindex = i-1;
//     } else {
//         currentnodeindex = i;
//     }
// 
//     if (direction == RIGHT) {
// #ifdef DEBUG
//         printf("merge:RIGHT\n");
// #endif
//         for (i = neighbornode->key_length-1; 0 <= i; i--) {
//             neighbornode->keys[i + currentnode->key_length + 1] = neighbornode->keys[i];
//         }
//         for (i = neighbornode->key_length; 0 <= i; i--) {
//             neighbornode->children[i + currentnode->key_length + 1] = neighbornode->children[i];
//         }
//         neighbornode->keys[currentnode->key_length] = anchornode->keys[currentnodeindex];
//         for (i = 0; i < currentnode->key_length; i++) {
//             neighbornode->keys[i] = currentnode->keys[i];
//         }
//         for (i = 0; i <= currentnode->key_length; i++) {
//             neighbornode->children[i] = currentnode->children[i];
//         }
//         anchornode->keys[currentnodeindex] = neighbornode->keys[0];
//         neighbornode->key_length += currentnode->key_length+1;
// 
//         result = currentnode;
//     } else {
// #ifdef DEBUG
//         printf("merge:LEFT\n");
// #endif
//         neighbornode->keys[neighbornode->key_length] = anchornode->keys[currentnodeindex];
//         for (i = 0; 0 < currentnode->key_length; i++) {
//             neighbornode->keys[neighbornode->key_length+1+i] = currentnode->keys[i];
//             neighbornode->children[neighbornode->key_length+i] = currentnode->children[i];
//         }
//         neighbornode->children[neighbornode->key_length+i] = currentnode->children[i];
//         anchornode->keys[currentnodeindex] = neighbornode->keys[0];
//         neighbornode->key_length += currentnode->key_length+1;
// 
//         result = currentnode;
//     }
// #ifdef DEBUG
//     printf("merge:complete.\n");
//     showInternalNode(anchornode, 0);
// #endif
// 
//     return result;
// }
// 
// void *rebalance(BPTree *bpt, void *currentnode, void *leftnode, void *rightnode, void *leftanchor, void *rightanchor, void *parent, void **balancenode) {
//     void *balancenodeanchor, *result;
//     char currentnodeisleaf = isPMEM(currentnode);
//     int nodesize = 0;
//     char direction = 0;
//     result = NULL;
// #ifdef DEBUG
//     printf("rebalance:balancing\n");
// #endif
// 
//     if (!currentnodeisleaf) {
//         if (leftnode == NULL) {
//             *balancenode = rightnode;
//             balancenodeanchor = rightanchor;
//             direction = RIGHT;
//         } else if (rightnode == NULL) {
//             *balancenode = leftnode;
//             balancenodeanchor = leftanchor;
//             direction = LEFT;
//         } else {
//             if (((InternalNode *)leftnode)->key_length > ((InternalNode *)rightnode)->key_length) {
//                 *balancenode = leftnode;
//                 balancenodeanchor = leftanchor;
//                 direction = LEFT;
//             } else {
//                 *balancenode = rightnode;
//                 balancenodeanchor = rightanchor;
//                 direction = RIGHT;
//             }
//         }
// 
//         nodesize = ((InternalNode *)(*balancenode))->key_length;
// 
//         if (nodesize > MIN_KEY) {
//             result = shift((InternalNode *)currentnode, (InternalNode *)(*balancenode), (InternalNode *)balancenodeanchor, direction, balancenode);
//         } else {
//             if (leftanchor == parent) {
//                 result = merge((InternalNode *)currentnode, (InternalNode *)leftnode, (InternalNode *)leftanchor, LEFT);
//             } else if (rightanchor == parent) {
//                 result = merge((InternalNode *)currentnode, (InternalNode *)rightnode, (InternalNode *)rightanchor, RIGHT);
//             }
//         }
//     } else {
//         *balancenode = currentnode; // need not null
//         deleteLeaf(bpt, (LeafNode *)currentnode, (LeafNode *)leftnode);
//         result = currentnode;
//     }
// 
//     return result;
// }
// 
// void *findRebalance(BPTree *bpt, void *currentnode, void *parent, void *leftnode, void *rightnode, void *leftanchor, void *rightanchor, Key targetkey, void **balancenode) {
//     void *removenode, *nextnode, *nextleft, *nextright, *nextleftanchor, *nextrightanchor, *result;
//     int key_length = 0;
//     int nextindex = -1;
//     int i;
//     char currentnodeisleaf = isPMEM(currentnode);
//     LeafNode *currentleaf = NULL;
//     InternalNode *currentinternal = NULL;
//     LeafNode *leftleaf = NULL;
//     InternalNode *leftinternal = NULL;
//     LeafNode *rightleaf = NULL;
//     InternalNode *rightinternal = NULL;
//     removenode = NULL;
//     nextnode = NULL;
//     nextleft = NULL;
//     nextright = NULL;
//     nextleftanchor = NULL;
//     nextrightanchor = NULL;
//     result = NULL;
// #ifdef DEBUG
//     printf("findrebalance:root=%p, currentnode=%p, leftnode=%p, rightnode=%p, leftanchor=%p, rightanchor=%p, targetkey=%ld\n", bpt->root, currentnode, leftnode, rightnode, leftanchor, rightanchor, targetkey);
// #endif
// 
//     if (currentnodeisleaf) {
//         currentleaf = (LeafNode *)currentnode;
//         leftleaf = (LeafNode *)leftnode;
//         rightleaf = (LeafNode *)rightnode;
// 
//         key_length = getLeafNodeLength(currentleaf);
// 
// #ifdef DEBUG
//         printf("findRebalance:leaf length = %d\n", key_length);
// #endif
// 
//         if (key_length > 1) {
//             *balancenode = NULL;
//         } else if (key_length <= MIN_KEY && *balancenode == NULL) {
//             *balancenode = currentnode;
//         }
//     } else {
//         currentinternal = (InternalNode *)currentnode;
//         leftinternal = (InternalNode *)leftnode;
//         rightinternal = (InternalNode *)rightnode;
// 
//         key_length = currentinternal->key_length;
// 
//         if (key_length > MIN_KEY) {
//             *balancenode = NULL;
//         } else if (key_length <= MIN_KEY && *balancenode == NULL) {
//             *balancenode = currentnode;
//         }
//     }
// 
//     if (currentnodeisleaf) {
//         SearchResult sr;
//         searchInLeaf(currentleaf, targetkey, &sr);
//         nextnode = sr.node;
//         nextindex = sr.index;
//     } else {
//         for (i = 0; i < currentinternal->key_length; i++) {
//             if (targetkey < currentinternal->keys[i]) {
//                 break;
//             }
//         }
//         nextnode = currentinternal->children[i];
//         nextindex = i;
//     }
// 
// #ifdef DEBUG
//     printf("next:%p[%d]\n", nextnode, nextindex);
// #endif
// 
//     if (!currentnodeisleaf) {
//         if (nextindex == 0) {
//             if (leftinternal != NULL) {
//                 nextleft = leftinternal->children[leftinternal->key_length-1];
//                 nextleftanchor = leftanchor;
//             }
//         } else {
//             nextleft = currentinternal->children[nextindex-1];
//             nextleftanchor = currentnode;
//         }
// 
//         if (nextindex == key_length) {
//             if (rightinternal != NULL) {
//                 nextright = rightinternal->children[0];
//                 nextrightanchor = rightanchor;
//             }
//         } else {
//             nextright = currentinternal->children[nextindex+1];
//             nextrightanchor = currentnode;
//         }
// 
//         removenode = findRebalance(bpt, nextnode, currentnode, nextleft, nextright, nextleftanchor, nextrightanchor, targetkey, balancenode);
// #ifdef DEBUG
//         printf("findrebalance:returned. removenode=%p\n", removenode);
// #endif
//     } else {
//         if (nextindex != -1) {
//             removenode = nextnode;
//         } else {
//             removenode = NULL;
//         }
//     }
// 
//     if (removenode == nextnode) {
// #ifdef DEBUG
//         printf("removing:%p\n", removenode);
// #endif
//         if (currentnodeisleaf) {
// #ifdef VALUE_NEEDS_FREE
//             PFree(currentleaf->kv[nextindex].value);
// #endif
//             CLR_BIT(currentleaf->header.bitmap, nextindex);
//             persist(currentleaf->header.bitmap + nextindex/8, sizeof(char));
// #ifdef DEBUG
//             printf("removed.\n");
//             showLeafNode(currentleaf, 0);
// #endif
//         } else {
//             if (isPMEM(currentinternal->children[nextindex])) {
//                 PFree(currentinternal->children[nextindex]);
//             } else {
//                 DFree(currentinternal->children[nextindex]);
//             }
//             if (nextindex == 0) {
//                 i = 0;
//             } else {
//                 i = nextindex-1;
//             }
//             for (; i < currentinternal->key_length-1; i++) {
//                 currentinternal->keys[i] = currentinternal->keys[i+1];
//             }
//             currentinternal->keys[currentinternal->key_length-1] = UNUSED_KEY;
// 
//             for (i = nextindex; i < currentinternal->key_length; i++) {
//                 currentinternal->children[i] = currentinternal->children[i+1];
//             }
//             currentinternal->children[currentinternal->key_length] = NULL;
//             currentinternal->key_length--;
//         }
//     }
// 
//     if (*balancenode == NULL) {
// #ifdef DEBUG
//         printf("delete:need not balancing\n");
// #endif
//         result = NULL;
//     } else if (currentnode == bpt->root) {
//         if (currentnodeisleaf) {
//             if (getLeafNodeLength(currentleaf) <= 0) {
//                 result = collapseRoot(currentnode);
//             }
//         } else {
//             if (currentinternal->key_length <= 0) {
//                 result = collapseRoot(currentnode);
//             }
//         }
//     } else {
//         result = rebalance(bpt, currentnode, leftnode, rightnode, leftanchor, rightanchor, parent, balancenode);
//     }
// 
//     return result;
// }
// 
// void delete(BPTree *bpt, Key targetkey) {
//     void *balancenode;
//     SearchResult sr;
//     search(&bpt->root, targetkey, &sr);
//     if (sr.index == -1) {
// #ifdef DEBUG
//         printf("delete:the key doesn't exist. abort.\n");
// #endif
//         return;
//     }
//     balancenode = NULL;
//     void *rv = findRebalance(bpt, bpt->root, NULL, NULL, NULL, NULL, NULL, targetkey, &balancenode);
//     if (rv != NULL) {
//         bpt->root = rv;
//     }
// }
// 
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
