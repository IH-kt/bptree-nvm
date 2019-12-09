#ifndef TREE_H
#define TREE_H
#  ifdef __cplusplus
extern "C" {
#  endif

#include <assert.h>
#include <pthread.h>

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

#ifdef TRANSACTION_SIZE
extern __thread unsigned int transaction_counter;
extern __thread unsigned int transaction_counter_max;
#  define COUNTUP_TRANSACTION_SIZE() {\
        transaction_counter++;\
    }
#  define UPDATE_TRANSACTION_SIZE() {\
        if (transaction_counter > transaction_counter_max)\
            transaction_counter_max = transaction_counter;\
        transaction_counter = 0;\
    }
#  define SHOW_TRANSACTION_SIZE() {\
        fprintf(stderr, "max of transaction size = %u\n", transaction_counter_max);\
    }
#else
#  define COUNTUP_TRANSACTION_SIZE()
#  define UPDATE_TRANSACTION_SIZE()
#  define SHOW_TRANSACTION_SIZE()
#endif

#ifdef FREQ_WRITE
#  define FREQ_WRITE_BUFSZ 60 * 60 * 16
#  define FREQ_INTERVAL 256 * 1024
static unsigned int wrote_size_tmp = 0;
static char freq_write_buf[FREQ_WRITE_BUFSZ];
static int freq_write_buf_index = 0;
#  define WRITE_FREQ_LOG(time) {\
   if (freq_write_buf_index + 16 <= FREQ_WRITE_BUFSZ) {\
     sprintf(freq_write_buf + freq_write_buf_index, "%15lf\n", time);\
     freq_write_buf_index += 16;\
   }\
}
#  define WRITE_FREQ_ADD(sz) {\
      unsigned int tmp = __sync_fetch_and_add(&wrote_size_tmp, (sz));\
      if (tmp + (sz) > FREQ_INTERVAL) {\
          if (__sync_bool_compare_and_swap(&wrote_size_tmp, tmp + (sz), 0)) {\
              struct timespec tm;\
              double time_tmp = 0;\
              clock_gettime(CLOCK_MONOTONIC_RAW, &tm);\
              time_tmp += tm.tv_nsec;  \
              time_tmp /= 1000000000;                   \
              time_tmp += tm.tv_sec;      \
              WRITE_FREQ_LOG(time_tmp);\
          }\
      }\
  }
#  define SHOW_FREQ_WRITE() {\
     FILE *ofile = fopen("write_freq.txt", "w");\
     if (ofile == NULL) {\
         perror("SHOW_FREQ_WRITE");\
     } else {\
         freq_write_buf[freq_write_buf_index] = '\0';\
         fprintf(ofile, "%s", freq_write_buf);/*fprintf(stderr, "%s", freq_write_buf);*/\
         fclose(ofile);\
     }\
   }

#else
#  define WRITE_FREQ_ADD(sz)
#  define SHOW_FREQ_WRITE()
#endif

#ifdef COUNT_WRITE 
static unsigned long nvm_write_count = 0;

#  define WRITE_COUNT_UP() ({\
      __sync_fetch_and_add(&nvm_write_count, 1);\
   })
#  define GET_WRITE_COUNT() (nvm_write_count)
#else
#  define WRITE_COUNT_UP()
#  define GET_WRITE_COUNT() (0l)
#endif

#define NVM_WRITE(p, v) ({\
      WRITE_COUNT_UP();\
      WRITE_FREQ_ADD(sizeof(v));\
      (*p = v);\
  })

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
