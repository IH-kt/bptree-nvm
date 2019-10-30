#ifndef FPTREE_H
#define FPTREE_H

#include <stdlib.h>
#include <stdio.h>
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

// #define MIN_KEY 124
#define MIN_KEY 2
#define MIN_DEG (MIN_KEY+1)
#define MAX_KEY (2*MIN_KEY)
#define MAX_DEG (MAX_KEY+1)
// #define MAX_PAIR 45
#define MAX_PAIR 3
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
struct InternalNode {
    int key_length;
    unsigned char children_type;
    struct InternalNode *parent;
    Key keys[MAX_KEY];
    void *children[MAX_DEG];
};

struct LeafHeader {
    unsigned char bitmap[BITMAP_SIZE];
    ppointer next;
    unsigned char fingerprints[MAX_PAIR];
};

struct LeafNode {
    struct LeafHeader header;
    KeyValuePair kv[MAX_PAIR];
    unsigned char lock;
};

struct BPTree {
    struct InternalNode *root;
    ppointer phead;
    struct LeafNode *head;
};

struct SearchResult {
    struct LeafNode *node;
    int index;
};

struct KeyPositionPair {
    Key key;
    int position;
};

typedef struct LeafNode LeafNode;
typedef struct InternalNode InternalNode;
typedef struct BPTree BPTree;
typedef struct SearchResult SearchResult;
typedef struct KeyPositionPair KeyPositionPair;

/* utils */
unsigned char hash(Key);
char popcntcharsize(char);

/* initializer */
void initKeyValuePair(KeyValuePair *);
void initLeafNode(LeafNode *);
void initInternalNode(InternalNode *);
void initBPTree(BPTree *, LeafNode *, InternalNode *);
void initSearchResult(SearchResult *);

LeafNode *newLeafNode();
void destroyLeafNode(LeafNode *);
InternalNode *newInternalNode();
void destroyInternalNode(InternalNode *);
BPTree *newBPTree();
void destroyBPTree(BPTree *);

int getLeafNodeLength(LeafNode *);
int searchInLeaf(LeafNode *, Key);
LeafNode *findLeaf(InternalNode *, Key, InternalNode **);
void search(BPTree *, Key, SearchResult *);
// 
// int findFirstAvailableSlot(LeafNode *);
int compareKeyPositionPair(const void *, const void *);
void updateParent(BPTree *, InternalNode *, Key, LeafNode *);
int updateUpward(InternalNode *, Key, LeafNode *, Key *, InternalNode **);
// void findSplitKey(LeafNode *, int *, char *);
// int newSplittedLeaf(BPTree *, InternalNode *, LeafNode *);
// InternalNode *newSplittedInternal(InternalNode *, InternalNode *);
// void insertNewKeyAndChild(InternalNode *, int, Key, void *);
// void insertNonfull(BPTree *, void *, KeyValuePair);
void insert(BPTree *, KeyValuePair);
// 
// void deleteLeaf(BPTree *, LeafNode *, LeafNode *);
// void *collapseRoot(void *);
// InternalNode *shift(InternalNode *, InternalNode *, InternalNode *, char, void **);
// InternalNode *merge(InternalNode *, InternalNode *, InternalNode *, char);
// void *rebalance(BPTree *, void *, void *, void *, void *, void *, void *, void **);
// void *findRebalance(BPTree *, void *, void *, void *, void *, void *, void *, Key, void **);
// void delete(BPTree *, Key);
// 
/* debug function */
void showLeafNode(LeafNode *, int);
void showInternalNode(InternalNode *, int);
void showTree(BPTree *);
#endif
