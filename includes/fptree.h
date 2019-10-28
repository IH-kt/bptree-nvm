#ifndef FPTREE_H
#define FPTREE_H

#include <stdlib.h>
#include <stddef.h>
#ifdef USE_ASSERTION
#  include <assert.h>
#endif
#ifndef NPERSIST
#  include <x86intrin.h>
#endif

#ifdef PMDK
#  include "allocator_pmdk.h"
#else
#  include "allocator.h"
#endif

#define MIN_KEY 124
#define MIN_DEG (MIN_KEY+1)
#define MAX_KEY (2*MIN_KEY)
#define MAX_DEG (MAX_KEY+1)
#define MAX_PAIR 45
#define BITMAP_SIZE ((MAX_PAIR/8)+1)

/* definition of structs */
/* value should be NULL and key must be 0 when pair is unused.
 * valid key should be larger than 0.
 */
typedef long Key;
#define UNUSED_KEY -1
typedef int Value;
#define INITIAL_VALUE 0

/* bitmap operator */
#define GET_BIT(bitmapaddr, index) (\
    (bitmapaddr[index/8] & (1 << ((index)%8))) >> (index)%8\
)
#define SET_BIT(bitmapaddr, index) (\
    bitmapaddr[index/8] |= (1 << ((index)%8)) \
)
#define CLR_BIT(bitmapaddr, index) (\
    bitmapaddr[index/8] &= ~(1 << ((index)%8)) \
)

/* structs */
typedef struct KeyValuePair {
    Key key;
    Value value;
} KeyValuePair;

#define LEAF 0
#define INTERNAL 1
struct InternalNodes {
    int key_length;
    unsigned char children_type;
    Key keys[MAX_KEY];
    void *children[MAX_DEG];
};

struct LeafHeader {
    unsigned char bitmap[BITMAP_SIZE];
    ppointer next;
    unsigned char fingerprints[MAX_PAIR];
};

struct LeafNodes {
    struct LeafHeader header;
    KeyValuePair kv[MAX_PAIR];
    unsigned char lock;
};

struct BPTree {
    struct InternalNodes *root;
    ppointer phead;
    struct LeafNodes *head;
};

struct SearchResult {
    void *nodes;
    int index;
};

typedef struct LeafNodes LeafNodes;
typedef struct InternalNodes InternalNodes;
typedef struct BPTree BPTree;
typedef struct SearchResult SearchResult;

/* utils */
unsigned char hash(Key);
char popcntcharsize(char);

/* initializer */
void initKeyValuePair(KeyValuePair *);
void initLeafNodes(LeafNodes *);
void initInternalNodes(InternalNodes *);
void initBPTree(BPTree *, LeafNodes *, InternalNodes *);
void initSearchResult(SearchResult *);

LeafNodes *newLeafNodes();
void destroyLeafNodes(LeafNodes *);
InternalNodes *newInternalNodes();
void destroyInternalNodes(InternalNodes *);
BPTree *newBPTree();
void destroyBPTree(BPTree *);

int getLeafNodeLength(LeafNodes *);
// void searchInLeaf(LeafNodes *, Key, SearchResult *);
// void search(void *, int, SearchResult *);
// 
// int findFirstAvailableSlot(LeafNodes *);
// int compare_pospair(const void *, const void *);
// void findSplitKey(LeafNodes *, int *, char *);
// int newSplittedLeaf(BPTree *, InternalNodes *, LeafNodes *);
// InternalNodes *newSplittedInternal(InternalNodes *, InternalNodes *);
// void insertNewKeyAndChild(InternalNodes *, int, Key, void *);
// void insertNonfull(BPTree *, void *, KeyValuePair);
// void insert(BPTree *, KeyValuePair);
// 
// void deleteLeaf(BPTree *, LeafNodes *, LeafNodes *);
// void *collapseRoot(void *);
// InternalNodes *shift(InternalNodes *, InternalNodes *, InternalNodes *, char, void **);
// InternalNodes *merge(InternalNodes *, InternalNodes *, InternalNodes *, char);
// void *rebalance(BPTree *, void *, void *, void *, void *, void *, void *, void **);
// void *findRebalance(BPTree *, void *, void *, void *, void *, void *, void *, Key, void **);
// void delete(BPTree *, Key);
// 
// /* debug function */
// void showLeafNodes(LeafNodes *, int);
// void showInternalNodes(InternalNodes *, int);
// void showTree(BPTree *);
#endif
