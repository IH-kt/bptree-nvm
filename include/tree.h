#ifndef TREE_H
#define TREE_H
#  ifdef __cplusplus
extern "C" {
#  endif

#include <assert.h>

#define MIN_KEY 124
#define MIN_DEG (MIN_KEY+1)
#define MAX_KEY (2*MIN_KEY+1)
#define MAX_DEG (MAX_KEY+1)
#define MAX_PAIR 45

/* definition of structs */
/* value should be NULL and key must be 0 when pair is unused.
 * valid key should be larger than 0.
 */
typedef long Key;
typedef int Value;
#ifdef NVHTM
extern const Key UNUSED_KEY;
extern const Value INITIAL_VALUE;
#else
#  define UNUSED_KEY -1
#  define INITIAL_VALUE 0
#endif

#ifdef COUNT_WRITE 
    static unsigned long nvm_write_count = 0;
#  define WRITE_COUNT_UP() (nvm_write_count++)
#  define NVM_WRITE(p, v) ({\
        WRITE_COUNT_UP();\
        *p = v;\
    })
#  define GET_WRITE_COUNT() (nvm_write_count)
#else
#  define WRITE_COUNT_UP()
#  define NVM_WRITE(p, v) (*p = v)
#  define GET_WRITE_COUNT() (0)
#endif

/* structs */
typedef struct KeyValuePair {
    Key key;
    Value value;
} KeyValuePair;

#ifdef BPTREE
#  include "bptree.h"
#else
#  include "fptree.h"
#endif

BPTree *newBPTree();
void destroyBPTree(BPTree *, unsigned char);
void search(BPTree *, Key, SearchResult *, unsigned char);

int insert(BPTree *, KeyValuePair, unsigned char);
int bptreeUpdate(BPTree *, KeyValuePair, unsigned char);
int bptreeRemove(BPTree *, Key, unsigned char);

/* debug function */
void showTree(BPTree *, unsigned char);

#  ifdef __cplusplus
};
#  endif
#endif
