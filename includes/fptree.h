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
#ifdef CONCURRENT
#  include <immintrin.h>
#  include <sched.h>
#  define XABORT_STAT 0
#  define RETRY_NUM 5 // times of retry RTM. < 32
// #  define LOOP_RETRY_NUM 10 // times of retry GET_LOCK_LOOP
#  define TRANSACTION 1
#  define LOCK 0
#endif

#ifdef PMDK
#  include "allocator_pmdk.h"
#else
#  include "allocator.h"
#endif

// #define MIN_KEY 124
#define MIN_KEY 3
#define MIN_DEG (MIN_KEY+1)
#define MAX_KEY (2*MIN_KEY+1)
#define MAX_DEG (MAX_KEY+1)
// #define MAX_PAIR 45
#define MAX_PAIR 4
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
// #define LEAF_HEADER_SIZE (BITMAP_SIZE+sizeof(ppointer)+MAX_PAIR)
// #define LEAF_DATA_SIZE (sizeof(KeyValuePair)*MAX_PAIR+1)
// #define LEAF_SIZE (LEAF_HEADER_SIZE+LEAF_DATA_SIZE)

struct InternalNode {
    int key_length;
    unsigned char children_type;
    Key keys[MAX_KEY];
    void *children[MAX_DEG];
};

struct LeafHeader {
    unsigned char bitmap[BITMAP_SIZE];
    ppointer pnext;
    unsigned char fingerprints[MAX_PAIR];
};

struct PersistentLeafNode {
    struct LeafHeader header;
    KeyValuePair kv[MAX_PAIR];
    unsigned char lock;
};

struct LeafNode {
    struct PersistentLeafNode *pleaf;
    struct LeafNode *next;
    struct LeafNode *prev;
    int key_length;
};

struct BPTree {
    struct InternalNode *root;
    ppointer *pmem_head;
    struct LeafNode *head;
    int lock;
};

struct SearchResult {
    struct LeafNode *node;
    int index;
};

struct KeyPositionPair {
    Key key;
    int position;
};

typedef struct PersistentLeafNode PersistentLeafNode;
typedef struct LeafNode LeafNode;
typedef struct InternalNode InternalNode;
typedef struct BPTree BPTree;
typedef struct SearchResult SearchResult;
typedef struct KeyPositionPair KeyPositionPair;

/* utils */
unsigned char hash(Key);
// char popcntcharsize(char);

/* initializer */
void initKeyValuePair(KeyValuePair *);
void initLeafNode(LeafNode *);
void initInternalNode(InternalNode *);
void initBPTree(BPTree *, LeafNode *, InternalNode *, ppointer *);
void initSearchResult(SearchResult *);

LeafNode *newLeafNode();
void destroyLeafNode(LeafNode *);
InternalNode *newInternalNode();
void destroyInternalNode(InternalNode *);
BPTree *newBPTree();
void destroyBPTree(BPTree *);

// int getLeafNodeLength(LeafNode *);
int searchInLeaf(LeafNode *, Key);
LeafNode *findLeaf(InternalNode *, Key, InternalNode **, unsigned char *);
void search(BPTree *, Key, SearchResult *);

int findFirstAvailableSlot(LeafNode *);
int compareKeyPositionPair(const void *, const void *);
void findSplitKey(LeafNode *, Key *, char *);
Key splitLeaf(LeafNode *, KeyValuePair);
Key splitInternal(InternalNode *, InternalNode **, void *, Key);
void insertNonfullInternal(InternalNode *, Key, void *);
void insertNonfullLeaf(LeafNode *, KeyValuePair);
int insert(BPTree *, KeyValuePair);
int searchNodeInInternalNode(InternalNode *, void *);
void insertParent(BPTree *, InternalNode *, Key, LeafNode *, LeafNode *);
int insertRecursive(InternalNode *, Key, LeafNode *, Key *, InternalNode **);

// void deleteLeaf(BPTree *, LeafNode *, LeafNode *);
// void *collapseRoot(void *);
// InternalNode *shift(InternalNode *, InternalNode *, InternalNode *, char, void **);
// InternalNode *merge(InternalNode *, InternalNode *, InternalNode *, char);
// void *rebalance(BPTree *, void *, void *, void *, void *, void *, void *, void **);
// void *findRebalance(BPTree *, void *, void *, void *, void *, void *, void *, Key, void **);
int delete(BPTree *, Key);
// 
/* debug function */
void showLeafNode(LeafNode *, int);
void showInternalNode(InternalNode *, int);
void showTree(BPTree *);
#endif
