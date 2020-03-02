#ifndef NH_H
#define NH_H

#include "nh_globals.h"
#include "tm.h"
#include "nvhtm.h"

#include "htm_retry_template.h"

#ifdef __cplusplus
extern "C"
{
  #endif

  #undef BEFORE_HTM_BEGIN
  #define BEFORE_HTM_BEGIN(tid, budget) \
    /*TM_ts1 = rdtscp()*/

  #undef AFTER_HTM_COMMIT
  #define AFTER_HTM_COMMIT(tid, budget) \
    /*NH_tx_time += rdtscp() - TM_ts1*/

  #define BEFORE_TRANSACTION_i(tid, budget) /* empty */
  #define AFTER_TRANSACTION_i(tid, budget)  /* empty */

  #undef BEFORE_TRANSACTION
  #define BEFORE_TRANSACTION(tid, budget) \
  BEFORE_TRANSACTION_i(tid, budget)

  #undef AFTER_TRANSACTION
  #define AFTER_TRANSACTION(tid, budget) \
  /*TM_ts2 = rdtscp();*/ \
  AFTER_TRANSACTION_i(tid, budget); \
  /*TM_ts3 = rdtscp(); \
  NH_count_writes += LOG_nb_writes; \
  NVHTM_stats_add_time(TM_ts2 - TM_ts1, TM_ts3 - TM_ts2); \
  NVHTM_thr_snapshot();*/

#ifdef STAT
  #undef AFTER_SGL_BEGIN
  #define AFTER_SGL_BEGIN(tid) \
  TM_inc_fallback(TM_tid_var)
#endif

  #define NH_begin() HTM_SGL_begin()
  // #define NH_begin_spec() HTM_SGL_begin_spec()

  #define NH_commit() HTM_SGL_commit()

  #undef HTM_THR_INIT
  #define HTM_THR_INIT() \
  HTM_SGL_tid = TM_tid_var

#ifdef STAT
  #undef HTM_INC
  #define HTM_INC(status) \
  TM_inc_error(TM_tid_var, status)
#endif

  // TODO: create wrapper to HTM_SGL_*

  #define NH_before_write(addr, val) /* empty */
  #define NH_after_write(addr, val)  /* empty */

  #define NH_write(addr, val) ({ \
    GRANULE_TYPE buf = val; \
    NH_before_write(addr, val); \
    /*MN_write*/memcpy(addr, &(buf), sizeof(GRANULE_TYPE), 0); /* *((GRANULE_TYPE*)addr) = val; */ \
    NH_after_write(addr, val); \
    val; \
  })

  extern int some_array[64][128];
  extern __thread int some_array_idx;

  #define NH_write_D(addr, val) ({ \
    GRANULE_TYPE g = CONVERT_GRANULE_D(val); \
    NH_write((GRANULE_TYPE*)addr, g); \
    val; \
  })

  #define NH_write_P(addr, val) ({ \
    GRANULE_TYPE g = (GRANULE_TYPE) val; /* works for pointers only */ \
    NH_write((GRANULE_TYPE*)addr, g); \
    val; \
  })

  #define NH_before_read(addr) /* empty */

  #define NH_read(addr) ({ \
    NH_before_read(addr); \
    (__typeof__(*addr))*(addr); \
  })

  #define NH_read_P(addr) ((__typeof__(*addr))NH_read(addr))
  #define NH_read_D(addr) ((__typeof__(*addr))NH_read(addr))

  /* TODO: persistency assumes an identifier */
  #define NH_alloc(size) malloc(size)
  #define NH_free(pool) free(pool)

  #ifdef __cplusplus
}
#endif

#include "nh_sol.h"

#endif /* NH_H */
