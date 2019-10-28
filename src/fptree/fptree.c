#include "fptree.h"
#include "allocator.h"

#ifndef NPERSIST
void persist(vpointer target, size_t size) {
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
void persist(vpointer target, size_t size) { /* EMPTY */ }
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

void initLeafNodes(LeafNodes *nodes) {
    int i;
    for (i = 0; i < BITMAP_SIZE; i++) {
        nodes->header.bitmap[i] = 0;
    }
    nodes->header.next = P_NULL;
    for (i = 0; i < MAX_PAIR; i++) {
        nodes->header.fingerprints[i] = 0;
        initKeyValuePair(&nodes->kv[i]);
    }
    nodes->lock = 0;
}

void initInternalNodes(InternalNodes *nodes) {
    int i;
    nodes->key_length = 0;
    nodes->children_type = LEAF;
    for (i = 0; i < MAX_KEY; i++) {
        nodes->keys[i] = UNUSED_KEY;
    }
    for (i = 0; i < MAX_DEG; i++) {
        nodes->children[i] = NULL;
    }
}

void initBPTree(BPTree *tree, LeafNodes *leafHead, InternalNodes *rootNode) {
    tree->phead = getPersistentAddr(leafHead);
    tree->head = leafHead;
    tree->root = rootNode;
    rootNode->children_type = LEAF;
    rootNode->children[0] = leafHead;
}

void initSearchResult(SearchResult *sr) {
    sr->nodes = NULL;
    sr->index = -1;
}

LeafNodes *newLeafNodes() {
    LeafNodes *new = (LeafNodes *)pmem_allocate(sizeof(LeafNodes));
    initLeafNodes(new);
    return new;
}
void destroyLeafNodes(LeafNodes *nodes) {
    pmem_free(nodes);
}

InternalNodes *newInternalNodes() {
    InternalNodes *new = (InternalNodes *)vmem_allocate(sizeof(InternalNodes));
    initInternalNodes(new);
    return new;
}
void destroyInternalNodes(InternalNodes *nodes) {
    vmem_free(nodes);
}

BPTree *newBPTree() {
    BPTree *new = (BPTree *)root_allocate(sizeof(BPTree), sizeof(LeafNodes));
    LeafNodes *headLeaf = newLeafNodes();
    InternalNodes *rootNode = newInternalNodes();
    initBPTree(new, headLeaf, rootNode);
    return new;
}
void destroyBPTree(BPTree *tree) {
    pmem_free(tree->head);
    vmem_free(tree->root);
    root_free(tree);
}

// int getLeafNodeLength(LeafNodes *ln) {
//     int keylength = 0, i;
//     for (i = 0; i < BITMAP_SIZE-1; i++) {
//         keylength += popcntcharsize(ln->header.bitmap[i]);
//     }
//     keylength += popcntcharsize(ln->header.bitmap[i] & (0xff >> (8-MAX_PAIR%8)));
// #ifdef DEBUG
//     printf("getLeafNodeLength:%p's keylength = %d\n", ln, keylength);
// #endif
//     return keylength;
// }
// 
// void searchInLeaf(LeafNodes *nodes, Key key, SearchResult *sr) {
//     int i;
//     unsigned char key_fp = hash(key);
//     initSearchResult(sr);
// #ifdef DEBUG
//     printf("searching in leaf\n");
// #endif
//     for (i = 0; i < MAX_PAIR; i++) {
// #ifdef DEBUG
// //        printf("%d:bitmap:%d, fingerprint:%d, key:%ld\n", i, GET_BIT(nodes->header.bitmap, i), nodes->header.fingerprints[i], nodes->kv[i].key);
// #endif
//         if (GET_BIT(nodes->header.bitmap, i) != 0 &&
//                 nodes->header.fingerprints[i] == key_fp &&
//                 nodes->kv[i].key == key) {
//             sr->nodes = nodes;
//             sr->index = i;
// #ifdef DEBUG
//             printf("search: found\n");
// #endif
//             return;
//         }
//     }
// #ifdef DEBUG
//     printf("search: not found\n");
// #endif
//     return;
// }
// 
// /* search for targetkey in btree.
//  * if it was not found, returns -1 index.
//  * otherwise, returns nodelist and index of target pair.
//  * destroySearchResult should be called after use.
//  */
// unsigned int count = 0;
// void search(void *current, int targetkey, SearchResult *sr) {
//     int i;
//     count++;
//     initSearchResult(sr);
// 
// #ifdef DEBUG
//     printf("search: key = %d\n", targetkey);
// #endif
// 
//     if (current == NULL) {
// #ifdef DEBUG
//         printf("search: empty\n");
// #endif
//     } else if (isPMEM(current)) {
//         // leaf node
//         searchInLeaf((LeafNodes *)current, targetkey, sr);
//     } else {
//         InternalNodes *current_int = (InternalNodes *)current;
//         for (i = 0; i < current_int->keylength; i++) {
//             if (targetkey < current_int->keys[i]) {
//                 break;
//             }
//         }
// 
// #ifdef DEBUG
//         if (i == current_int->keylength) {
//             printf("search: reached end of list. go to %ld's right child.\n", current_int->keys[i-1]);
//         } else {
//             printf("search: go to %ld's left child.\n", current_int->keys[i]);
//         }
// #endif
// 
//         current = current_int->children[i];
//         search(current, targetkey, sr);
//     }
// }
// 
// int findFirstAvailableSlot(LeafNodes *target) {
//     int i;
//     for (i = 0; i < MAX_PAIR; i++) {
//         if (GET_BIT(target->header.bitmap, i) == 0) {
//             break;
//         }
//     }
//     return i;
// }
// 
// int compare_pospair(const void *a, const void *b) {
//     return ((SplitPosPair *)a)->key - ((SplitPosPair *)b)->key;
// }
// 
// void findSplitKey(LeafNodes *target, int *sppos, char *bitmap) {
//     int i;
//     SplitPosPair keys[MAX_PAIR];
// 
//     // キーは全て有効（満杯の葉が入ってくるので）
//     for (i = 0; i < MAX_PAIR; i++) {
//         keys[i].key = target->kv[i].key;
//         keys[i].orgpos = i;
//     }
// 
//     qsort(keys, MAX_PAIR, sizeof(SplitPosPair), compare_pospair);
// #ifdef DEBUG
//     for (i = 0; i < MAX_PAIR; i++) {
//         printf("sorted[%d] = %ld\n", i, keys[i].key);
//     }
// #endif
//     
// #ifdef USE_ASSERTION
//     assert( MAX_PAIR & 1 == 1 );
// #endif
// 
//     *sppos = keys[MAX_PAIR/2].orgpos;
//     // MAX_PAIRは奇数とする
//     for (i = MAX_PAIR/2; i < MAX_PAIR; i++) {
//         SET_BIT(bitmap, keys[i].orgpos);
//     }
// #ifdef DEBUG
//     printf("leaf splitted at %d\n", *sppos);
//     for (i = 0; i < BITMAP_SIZE; i++) {
//         printf("bitmap[%d] = %x\n", i, bitmap[i]);
//     }
// #endif
// }
// 
// int newSplittedLeaf(BPTree *bpt, InternalNodes *parent, LeafNodes *target) {
//     SplitLog *log = &bpt->splitlog.splitlog;
//     log->pcurrentleaf = getPersistentAddr(target);
//     persist(&target, sizeof(LeafNodes *));
//     log->pnewleaf = getPersistentAddr(newLeafNodes());
//     LeafNodes *nlnodes = getTransientAddr(log->pnewleaf);
//     int i;
//     int sppos;
//     char bitmap[BITMAP_SIZE];
// 
//     for (i = 0; i < MAX_PAIR; i++) {
//         nlnodes->kv[i] = target->kv[i];
//         nlnodes->header.fingerprints[i] = target->header.fingerprints[i];
//     }
//     nlnodes->header.next = target->header.next;
//     nlnodes->lock = target->lock;
//     persist(nlnodes, sizeof(LeafNodes));
// 
//     findSplitKey(nlnodes, &sppos, bitmap);
// 
//     for (i = 0; i < BITMAP_SIZE; i++) {
//         nlnodes->header.bitmap[i] = bitmap[i];
//     }
//     persist(nlnodes->header.bitmap, BITMAP_SIZE * sizeof(bitmap[0]));
// 
//     for (i = 0; i < BITMAP_SIZE; i++) {
//         target->header.bitmap[i] = ~bitmap[i];
//     }
//     persist(target->header.bitmap, BITMAP_SIZE * sizeof(bitmap[0]));
// 
//     target->header.next = getPersistentAddr(nlnodes);
// 
// #ifdef DEBUG
//     showSplitLog(log);
// #endif
//     resetMicroLog(&bpt->splitlog);
//     return sppos;
// }
// 
// InternalNodes *newSplittedInternal(InternalNodes *parent, InternalNodes *target) {
//     int i;
//     InternalNodes *ninodes = newInternalNodes();
//     ninodes->keylength = MAX_KEY/2;
// 
//     for (i = 0; i < MAX_KEY/2; i++) {
//         ninodes->keys[i] = target->keys[i+(MAX_KEY)/2+1];
//         ninodes->children[i] = target->children[i+(MAX_KEY)/2+1];
//         target->keys[i+(MAX_KEY)/2+1] = 0;
//         target->children[i+(MAX_KEY)/2+1] = NULL;
//     }
//     ninodes->children[i] = target->children[i+(MAX_KEY)/2+1];
// 
//     target->keylength = MAX_KEY/2;
// 
//     return ninodes;
// }
// 
// /* targetIndexは挿入するキーの位置 */
// void insertNewKeyAndChild(InternalNodes *target, int targetIndex, Key key, void *child) {
//     int i;
//     for (i = target->keylength; targetIndex < i; i--) {
//         target->children[i+1] = target->children[i];
//     }
//     target->children[targetIndex+1] = child;
// 
//     for (i = target->keylength-1; targetIndex <= i; i--) {
//         target->keys[i+1] = target->keys[i];
//     }
//     target->keys[targetIndex] = key;
//     target->keylength++;
// }
// 
// void insertNonfull(BPTree *bpt, void *nodes, KeyValuePair kv) {
//     int i;
// #ifdef DEBUG
//     printf("insertNonfull:%ld -> %p\n", kv.key, nodes);
// #endif
//     if (isPMEM(nodes)) {
//         LeafNodes *nodes_lf = (LeafNodes *)nodes;
// #ifdef DEBUG
//         printf("insertNonfull:leaf\n");
// #endif
//         int slot = findFirstAvailableSlot(nodes_lf);
// 
//         nodes_lf->kv[slot] = kv;
//         nodes_lf->header.fingerprints[slot] = hash(kv.key);
//         persist(&nodes_lf->kv[slot], sizeof(KeyValuePair));
//         persist(&nodes_lf->header.fingerprints[slot], sizeof(char));
//         SET_BIT(nodes_lf->header.bitmap, slot);
//         persist(&nodes_lf->header.bitmap[slot/8], sizeof(char));
//     } else {
// #ifdef DEBUG
//         printf("insertNonfull:internal\n");
// #endif
//         InternalNodes *nodes_int = (InternalNodes *)nodes;
//         for (i = nodes_int->keylength-1; 0 <= i; i--) {
//             if (kv.key >= nodes_int->keys[i]) {
//                 break;
//             }
//         }
//         i++;
// 
//         int childlength;
//         if (isPMEM(nodes_int->children[i])) {
//             LeafNodes *target = (LeafNodes *)nodes_int->children[i];
//             if (findFirstAvailableSlot(target) == MAX_PAIR) {
//                 int splitpos = newSplittedLeaf(bpt, nodes_int, target);
//                 LeafNodes *newNode = getTransientAddr(target->header.next);
//                 insertNewKeyAndChild(nodes_int, i, newNode->kv[splitpos].key, newNode);
//                 if (kv.key >= nodes_int->keys[i]) {
//                     i++;
//                 }
//             }
//         } else {
//             InternalNodes *target = (InternalNodes *)nodes_int->children[i];
//             if (target->keylength == MAX_KEY) {
//                 InternalNodes *newNode = newSplittedInternal(nodes_int, target);
// 
//                 insertNewKeyAndChild(nodes_int, i, target->keys[MAX_KEY/2], newNode);
//                 target->keys[MAX_KEY/2] = UNUSED_KEY;
//                 if (kv.key >= nodes_int->keys[i]) {
//                     i++;
//                 }
//             }
//         }
// 
//         insertNonfull(bpt, nodes_int->children[i], kv);
//     }
// }
// 
// // TODO: 下りながらsplitするのをやめる
// void insert(BPTree *bpt, KeyValuePair kv) {
//     void *root = bpt->root;
//     char full = 0;
//     SearchResult sr;
//     search(root, kv.key, &sr);
//     if (sr.index != -1) {
// #ifdef DEBUG
//         printf("key exist. abort.\n");
// #endif
//         return;
//     }
// 
// #ifdef DEBUG
//     printf("inserting:%ld\n", kv.key);
//     printf("root:%p\n", bpt->root);
// #endif
// 
//     char ispmem = isPMEM(root);
//     if (ispmem) {
//         full = MAX_PAIR == findFirstAvailableSlot(root);
//     } else {
//         full = MAX_KEY == ((InternalNodes *)root)->keylength;
//     }
//     if (full) {
//         InternalNodes *newRoot = newInternalNodes();
// #ifdef DEBUG
//         printf("newroot:%p\n", newRoot);
// #endif
//         void *oldRoot = bpt->root;
//         bpt->root = newRoot;
//         newRoot->keylength = 0;
//         newRoot->children[0] = root;
// 
//         if (ispmem) {
//             LeafNodes *oldRootLeaf = (LeafNodes *)oldRoot;
//             int splitpos = newSplittedLeaf(bpt, newRoot, oldRootLeaf);
//             LeafNodes *newLeaf = (LeafNodes *)getTransientAddr(oldRootLeaf->header.next);
//             insertNewKeyAndChild(newRoot, 0, newLeaf->kv[splitpos].key, newLeaf);
//         } else {
//             InternalNodes *oldRootInternal = (InternalNodes *)oldRoot;
//             InternalNodes *newInternal = newSplittedInternal(newRoot, oldRootInternal);
//             insertNewKeyAndChild(newRoot, 0, oldRootInternal->keys[MAX_KEY/2], newInternal);
//             oldRootInternal->keys[MAX_KEY/2] = UNUSED_KEY;
//         }
// 
//         insertNonfull(bpt, newRoot, kv);
//     } else {
//         insertNonfull(bpt, bpt->root, kv);
//     }
// }
// 
// void deleteLeaf(BPTree *bpt, LeafNodes *current, LeafNodes *prev) {
//     DeleteLog *log = &bpt->deletelog.deletelog;
//     log->pcurrentleaf = getPersistentAddr(current);
//     persist(current, sizeof(LeafNodes *));
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
//         newroot = ((InternalNodes *)oldroot)->children[0];
//         DFree(oldroot);
//     }
//     return newroot;
// }
// 
// InternalNodes *shift(InternalNodes *currentnode, InternalNodes *neighbornode, InternalNodes *anchornode, char direction, void **balancenode) {
//     int i, shiftsize, currentnodeindex;
// #ifdef DEBUG
//     printf("shift:shifting\n");
// #endif
//     for (i = 0; i < anchornode->keylength; i++) {
//         if (currentnode->keys[0] < anchornode->keys[i]) {
//             break;
//         }
//     }
//     currentnodeindex = i;
//     shiftsize = (neighbornode->keylength - currentnode->keylength)/2;
//     if (direction == LEFT) {
//         for (i = currentnode->keylength-1; 0 <= i; i--) {
//             currentnode->keys[i+shiftsize] = currentnode->keys[i];
//         }
//         for (i = currentnode->keylength; 0 < i; i--) {
//             currentnode->children[i+shiftsize] = currentnode->children[i];
//         }
// 
//         currentnode->keys[shiftsize-1] = anchornode->keys[currentnodeindex-1];
//         for (i = 0; i < shiftsize-1; i++) {
//             currentnode->keys[shiftsize-2-i] = neighbornode->keys[neighbornode->keylength-1-i];
//             currentnode->children[shiftsize-1-i] = neighbornode->children[neighbornode->keylength-i];
//             neighbornode->keys[neighbornode->keylength-1-i] = UNUSED_KEY;
//             neighbornode->children[neighbornode->keylength-i] = NULL;
//         }
//         currentnode->children[0] = neighbornode->children[neighbornode->keylength-shiftsize+1];
//         neighbornode->children[neighbornode->keylength-shiftsize+1] = NULL;
// 
//         anchornode->keys[currentnodeindex-1] = neighbornode->keys[neighbornode->keylength-shiftsize];
//         neighbornode->keys[neighbornode->keylength-shiftsize] = UNUSED_KEY;
//     } else {
//         currentnode->keys[currentnode->keylength] = anchornode->keys[currentnodeindex];
//         for (i = 0; i < shiftsize-1; i++) {
//             currentnode->keys[currentnode->keylength+1+i] = neighbornode->keys[i];
//             currentnode->children[currentnode->keylength+1+i] = neighbornode->children[i];
//         }
//         currentnode->children[currentnode->keylength+shiftsize] = neighbornode->children[shiftsize-1];
//         anchornode->keys[currentnodeindex] = neighbornode->keys[shiftsize-1];
// 
//         for (i = 0; i + shiftsize < neighbornode->keylength; i++) {
//             neighbornode->keys[i] = neighbornode->keys[i + shiftsize];
//             neighbornode->children[i] = neighbornode->children[i + shiftsize];
//             neighbornode->keys[i + shiftsize] = UNUSED_KEY;
//             neighbornode->children[i + shiftsize] = NULL;
//         }
//         neighbornode->children[i] = neighbornode->children[i + shiftsize];
//         neighbornode->children[i + shiftsize] = NULL;
//     }
// 
//     neighbornode->keylength -= shiftsize;
//     currentnode->keylength += shiftsize;
// 
//     *balancenode = NULL;
// 
//     return NULL;
// }
// 
// InternalNodes *merge(InternalNodes *currentnode, InternalNodes *neighbornode, InternalNodes *anchornode, char direction) {
//     int i, currentnodeindex;
//     InternalNodes *result;
// #ifdef DEBUG
//     printf("merge:current=%p, neighbor=%p, anchor=%p\n", currentnode, neighbornode, anchornode);
//     showInternalNodes(anchornode, 0);
// #endif
//     for (i = 0; i < anchornode->keylength; i++) {
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
//         for (i = neighbornode->keylength-1; 0 <= i; i--) {
//             neighbornode->keys[i + currentnode->keylength + 1] = neighbornode->keys[i];
//         }
//         for (i = neighbornode->keylength; 0 <= i; i--) {
//             neighbornode->children[i + currentnode->keylength + 1] = neighbornode->children[i];
//         }
//         neighbornode->keys[currentnode->keylength] = anchornode->keys[currentnodeindex];
//         for (i = 0; i < currentnode->keylength; i++) {
//             neighbornode->keys[i] = currentnode->keys[i];
//         }
//         for (i = 0; i <= currentnode->keylength; i++) {
//             neighbornode->children[i] = currentnode->children[i];
//         }
//         anchornode->keys[currentnodeindex] = neighbornode->keys[0];
//         neighbornode->keylength += currentnode->keylength+1;
// 
//         result = currentnode;
//     } else {
// #ifdef DEBUG
//         printf("merge:LEFT\n");
// #endif
//         neighbornode->keys[neighbornode->keylength] = anchornode->keys[currentnodeindex];
//         for (i = 0; 0 < currentnode->keylength; i++) {
//             neighbornode->keys[neighbornode->keylength+1+i] = currentnode->keys[i];
//             neighbornode->children[neighbornode->keylength+i] = currentnode->children[i];
//         }
//         neighbornode->children[neighbornode->keylength+i] = currentnode->children[i];
//         anchornode->keys[currentnodeindex] = neighbornode->keys[0];
//         neighbornode->keylength += currentnode->keylength+1;
// 
//         result = currentnode;
//     }
// #ifdef DEBUG
//     printf("merge:complete.\n");
//     showInternalNodes(anchornode, 0);
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
//             if (((InternalNodes *)leftnode)->keylength > ((InternalNodes *)rightnode)->keylength) {
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
//         nodesize = ((InternalNodes *)(*balancenode))->keylength;
// 
//         if (nodesize > MIN_KEY) {
//             result = shift((InternalNodes *)currentnode, (InternalNodes *)(*balancenode), (InternalNodes *)balancenodeanchor, direction, balancenode);
//         } else {
//             if (leftanchor == parent) {
//                 result = merge((InternalNodes *)currentnode, (InternalNodes *)leftnode, (InternalNodes *)leftanchor, LEFT);
//             } else if (rightanchor == parent) {
//                 result = merge((InternalNodes *)currentnode, (InternalNodes *)rightnode, (InternalNodes *)rightanchor, RIGHT);
//             }
//         }
//     } else {
//         *balancenode = currentnode; // need not null
//         deleteLeaf(bpt, (LeafNodes *)currentnode, (LeafNodes *)leftnode);
//         result = currentnode;
//     }
// 
//     return result;
// }
// 
// void *findRebalance(BPTree *bpt, void *currentnode, void *parent, void *leftnode, void *rightnode, void *leftanchor, void *rightanchor, Key targetkey, void **balancenode) {
//     void *removenode, *nextnode, *nextleft, *nextright, *nextleftanchor, *nextrightanchor, *result;
//     int keylength = 0;
//     int nextindex = -1;
//     int i;
//     char currentnodeisleaf = isPMEM(currentnode);
//     LeafNodes *currentleaf = NULL;
//     InternalNodes *currentinternal = NULL;
//     LeafNodes *leftleaf = NULL;
//     InternalNodes *leftinternal = NULL;
//     LeafNodes *rightleaf = NULL;
//     InternalNodes *rightinternal = NULL;
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
//         currentleaf = (LeafNodes *)currentnode;
//         leftleaf = (LeafNodes *)leftnode;
//         rightleaf = (LeafNodes *)rightnode;
// 
//         keylength = getLeafNodeLength(currentleaf);
// 
// #ifdef DEBUG
//         printf("findRebalance:leaf length = %d\n", keylength);
// #endif
// 
//         if (keylength > 1) {
//             *balancenode = NULL;
//         } else if (keylength <= MIN_KEY && *balancenode == NULL) {
//             *balancenode = currentnode;
//         }
//     } else {
//         currentinternal = (InternalNodes *)currentnode;
//         leftinternal = (InternalNodes *)leftnode;
//         rightinternal = (InternalNodes *)rightnode;
// 
//         keylength = currentinternal->keylength;
// 
//         if (keylength > MIN_KEY) {
//             *balancenode = NULL;
//         } else if (keylength <= MIN_KEY && *balancenode == NULL) {
//             *balancenode = currentnode;
//         }
//     }
// 
//     if (currentnodeisleaf) {
//         SearchResult sr;
//         searchInLeaf(currentleaf, targetkey, &sr);
//         nextnode = sr.nodes;
//         nextindex = sr.index;
//     } else {
//         for (i = 0; i < currentinternal->keylength; i++) {
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
//                 nextleft = leftinternal->children[leftinternal->keylength-1];
//                 nextleftanchor = leftanchor;
//             }
//         } else {
//             nextleft = currentinternal->children[nextindex-1];
//             nextleftanchor = currentnode;
//         }
// 
//         if (nextindex == keylength) {
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
//             showLeafNodes(currentleaf, 0);
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
//             for (; i < currentinternal->keylength-1; i++) {
//                 currentinternal->keys[i] = currentinternal->keys[i+1];
//             }
//             currentinternal->keys[currentinternal->keylength-1] = UNUSED_KEY;
// 
//             for (i = nextindex; i < currentinternal->keylength; i++) {
//                 currentinternal->children[i] = currentinternal->children[i+1];
//             }
//             currentinternal->children[currentinternal->keylength] = NULL;
//             currentinternal->keylength--;
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
//             if (currentinternal->keylength <= 0) {
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
// /* debug function */
// void showLeafNodes(LeafNodes *nodes, int depth) {
//     int i, j;
//     for (j = 0; j < depth; j++)
//         printf("\t");
//     printf("leaf:%p\n", nodes);
//     for (j = 0; j < depth; j++)
//         printf("\t");
//     printf("\tnext = %p\n", getTransientAddr(nodes->header.next));
//     for (i = 0; i < MAX_PAIR; i++) {
//         for (j = 0; j < depth; j++)
//             printf("\t");
//         printf("\tbitmap[%d]:%d\n", i, GET_BIT(nodes->header.bitmap, i));
//     }
//     for (i = 0; i < MAX_PAIR; i++) {
//         if ((GET_BIT(nodes->header.bitmap, i)) == 0)
//             continue;
//         for (j = 0; j < depth; j++)
//             printf("\t");
//         printf("\tkv[%d]:fgp = %d, key = %ld, val = %d\n", i, nodes->header.fingerprints[i], nodes->kv[i].key, nodes->kv[i].value);
//     }
// }
// 
// void showInternalNodes(InternalNodes *nodes, int depth) {
//     int i, j;
//     for (j = 0; j < depth; j++)
//         printf("\t");
//     printf("internal:%p\n", nodes);
//     for (j = 0; j < depth; j++)
//         printf("\t");
//     printf("\tlength = %d\n", nodes->keylength);
//     for (i = 0; i < MAX_KEY; i++) {
//         for (j = 0; j < depth; j++)
//             printf("\t");
//         printf("\tkey[%d] = %ld\n", i, nodes->keys[i]);
//     }
//     for (i = 0; i < nodes->keylength+1; i++) {
//         for (j = 0; j < depth; j++)
//             printf("\t");
//         printf("\tchildren[%d]:\n", i);
//         if (isPMEM(nodes->children[i])) {
//             showLeafNodes((LeafNodes *)nodes->children[i], depth+1);
//         } else {
//             showInternalNodes((InternalNodes *)nodes->children[i], depth+1);
//         }
//     }
// }
// 
// void showTree(BPTree *bpt) {
//     if (isPMEM(bpt->root)) {
//         showLeafNodes((LeafNodes *)bpt->root, 0);
//     } else {
//         showInternalNodes((InternalNodes *)bpt->root, 0);
//     }
// }
