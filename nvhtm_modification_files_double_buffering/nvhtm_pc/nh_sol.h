#ifndef NH_SOL_H
#define NH_SOL_H

#ifdef __cplusplus
extern "C"
{
  #endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

  #define MAXIMUM_OFFSET 400 // in cycles

// TODO: remove the externs
  #undef BEFORE_HTM_BEGIN_spec
  #define BEFORE_HTM_BEGIN_spec(tid, budget) \
    extern __thread int global_threadId_; \
    extern int nb_transfers; \
    extern long nb_of_done_transactions; \
    while (*NH_checkpointer_state == 1 && nb_of_done_transactions < nb_transfers) { \
      PAUSE(); \
    }

#undef AFTER_SGL_BEGIN
#define AFTER_SGL_BEGIN(tid)                                                     \
  {                                                                               \
    int id = TM_tid_var;\
    TM_inc_fallback(tid);\
    persistent_checkpointing[id].flag = 1;                                            \
    struct timespec stt, edt;\
    clock_gettime(CLOCK_MONOTONIC_RAW, &stt);\
    while (*NH_checkpointer_state == 1) {                                        \
      PAUSE();                                                                    \
    }\
    clock_gettime(CLOCK_MONOTONIC_RAW, &edt); \
    double time_tmp = 0;                      \
    time_tmp += (edt.tv_nsec - stt.tv_nsec);  \
    time_tmp /= 1000000000;                   \
    time_tmp += edt.tv_sec - stt.tv_sec;      \
    NH_nanotime_blocked += time_tmp;  \
    /*printf("before_sgl_begin set:%d\n", tid);*/\
    __sync_synchronize();                                                         \
    nvm_htm_local_log = NH_global_logs[id];                                      \
	LOG_local_state.start = nvm_htm_local_log->start;                                                  \
	LOG_local_state.end = nvm_htm_local_log->end;                                                      \
	LOG_local_state.counter = distance_ptr((int)LOG_local_state.start,                   \
										   (int)LOG_local_state.end);                    \
    if ((LOG_local_state.size_of_log - LOG_local_state.counter) < 2048)            \
    {                                                                             \
        clock_gettime(CLOCK_MONOTONIC_RAW, &stt);\
      /*printf("sgl %d: not enough space -> %d - %d\n", tid, LOG_local_state.size_of_log, LOG_local_state.counter);*/                                  \
    /*printf("before_sgl_begin unset:%d\n", tid);*/\
      __sync_synchronize();                                                       \
      while (sem_trywait(NH_chkp_sem) != -1);\
      while ((LOG_local_state.size_of_log - LOG_local_state.counter) < 2048)       \
      {                                                                           \
          int sem_val;\
          if ((*NH_checkpointer_state) == 0) {\
              sem_getvalue(NH_chkp_sem, &sem_val);\
              if (sem_val <= 0) {\
                  sem_post(NH_chkp_sem);                                                      \
              }\
              while ((*NH_checkpointer_state) != 2) {\
                  PAUSE();\
              }\
          }\
          if ((*NH_checkpointer_state) == 2) {\
              persistent_checkpointing[id].flag = 0;\
              while ((*NH_checkpointer_state) == 2) {\
                  PAUSE();\
              }\
              persistent_checkpointing[id].flag = 1;\
          }\
          while ((*NH_checkpointer_state) == 1) {\
              PAUSE();\
          }\
          persistent_checkpointing[id].flag = 1;\
          nvm_htm_local_log = NH_global_logs[id];                                      \
          LOG_local_state.start = nvm_htm_local_log->start;                                                  \
          LOG_local_state.end = nvm_htm_local_log->end;                                                      \
          LOG_local_state.counter = distance_ptr((int)LOG_local_state.start,                   \
                  (int)LOG_local_state.end);                    \
          /*printf("after_sgl_begin\n");*/\
      }                                                                           \
      _mm_sfence();                                                               \
        persistent_checkpointing[id].flag = 1;\
    /*printf("before_sgl_begin2 set:%d\n", tid);*/\
      __sync_synchronize();                                                       \
      nvm_htm_local_log = NH_global_logs[id];                                    \
      LOG_local_state.start = nvm_htm_local_log->start;                                                  \
      LOG_local_state.end = nvm_htm_local_log->end;                                                      \
      LOG_local_state.counter = distance_ptr((int)LOG_local_state.start,                   \
              (int)LOG_local_state.end);                    \
      /*printf("sgl %d: exiting -> %d - %d\n", tid, LOG_local_state.size_of_log, LOG_local_state.counter)*/;                                  \
        clock_gettime(CLOCK_MONOTONIC_RAW, &edt); \
        double time_tmp = 0;                      \
        time_tmp += (edt.tv_nsec - stt.tv_nsec);  \
        time_tmp /= 1000000000;                   \
        time_tmp += edt.tv_sec - stt.tv_sec;      \
        NH_nanotime_blocked += time_tmp;  \
    }                                                                             \
  /*printf("AftrSGLBgn %d-%d: start = %d, end = %d, local start = %d, local end = %d, counter = %d\n", tid, id, NH_global_logs[id]->start, NH_global_logs[id]->end, LOG_local_state.start, LOG_local_state.end, LOG_local_state.counter);*/\
  }

#undef BEFORE_TRANSACTION_i
#define BEFORE_TRANSACTION_i(tid, budget)           \
  while ((*NH_checkpointer_state) == 2) { \
      if (persistent_checkpointing[TM_tid_var].flag)\
      persistent_checkpointing[TM_tid_var].flag = 0;\
  }        \
  LOG_get_ts_before_tx(tid);                        \
  LOG_before_TX();                                  \
  /*printf("BfrTrnsctn %d-%d: start = %d, end = %d, local start = %d, local end = %d, nvm_htm_local_log = %p\n", tid, TM_tid_var, NH_global_logs[TM_tid_var]->start, NH_global_logs[TM_tid_var]->end, LOG_local_state.start, LOG_local_state.end, nvm_htm_local_log);*/\
  TM_inc_local_counter(tid);

#undef BEFORE_COMMIT
#define BEFORE_COMMIT(tid, budget, status)             \
  persistent_checkpointing[TM_tid_var].flag = 1;                   \
    /*printf("before_commit set:%d\n", tid);*/\
  if (HTM_test() && (*NH_checkpointer_state) == 2) HTM_named_abort(2);\
  if (NH_global_logs[TM_tid_var] != nvm_htm_local_log || (*NH_checkpointer_state) == 1)                          \
  {                                                    \
    if (HTM_test()) HTM_named_abort(2);                \
    fprintf(stderr, "before_commit:%d-%d\n", TM_tid_var, tid);\
    assert(0); /* flipped during lock execution */     \
  }                                                    \
  ts_var = rdtscp(); /* must be the p version */       \
  if (LOG_count_writes(tid) > 0 && TM_nb_threads > 28) \
  {                                                    \
    while ((rdtscp() - ts_var) < MAXIMUM_OFFSET)       \
      ; /* wait offset */                              \
  }

#undef AFTER_TRANSACTION_i
#define AFTER_TRANSACTION_i(_tid, budget) ({                                   \
  /*printf("AftTrnsctn %d-%d: start = %d, end = %d, local start = %d, local end = %d, nvm_htm_local_log = %p\n", _tid, TM_tid_var, NH_global_logs[TM_tid_var]->start, NH_global_logs[TM_tid_var]->end, LOG_local_state.start, LOG_local_state.end, nvm_htm_local_log);*/\
  int id = TM_tid_var;\
  int nb_writes = LOG_count_writes(_tid);                                      \
  if (nb_writes)                                                              \
  {                                                                           \
    htm_tx_val_counters[_tid].global_counter = ts_var;                         \
    __sync_synchronize();                                                     \
    NVMHTM_commit(id, ts_var, nb_writes);                                    \
  }                                                                           \
  __sync_synchronize();                                                     \
  CHECK_AND_REQUEST(_tid);                                                     \
  TM_inc_local_counter(_tid);                                                  \
  LOG_after_TX();                                                             \
  assert(__sync_bool_compare_and_swap(&persistent_checkpointing[id].flag, 1, 0)); \
    /*printf("after_transaction unset:%d\n", _tid);*/\
})

#undef AFTER_ABORT
#define AFTER_ABORT(tid, budget, status)                                       \
  /* NH_tx_time += rdtscp() - TM_ts1; */                                       \
  {\
  struct timespec stt, edt;\
  clock_gettime(CLOCK_MONOTONIC_RAW, &stt); \
  while (*NH_checkpointer_state == 2) { \
      if (persistent_checkpointing[TM_tid_var].flag) {\
          persistent_checkpointing[TM_tid_var].flag = 0; \
      } \
    PAUSE(); \
  }        \
  clock_gettime(CLOCK_MONOTONIC_RAW, &edt); \
  double time_tmp = 0;                      \
  time_tmp += (edt.tv_nsec - stt.tv_nsec);  \
  time_tmp /= 1000000000;                   \
  time_tmp += edt.tv_sec - stt.tv_sec;      \
  NH_nanotime_blocked += time_tmp;  \
  }\
  /*printf("AbortState %d-%d: status = %x\n", tid, TM_tid_var, status);*/\
  CHECK_LOG_ABORT(tid, status);                                                \
  LOG_get_ts_before_tx(tid);                                                   \
  /*printf("AfterAbort %d-%d: start = %d, end = %d, local start = %d, local end = %d\n", tid, TM_tid_var, NH_global_logs[tid]->start, NH_global_logs[tid]->end, LOG_local_state.start, LOG_local_state.end);*/\
  __sync_synchronize();                                                        \
  ts_var = rdtscp();                                                           \
  htm_tx_val_counters[tid].global_counter = ts_var;                            \
  /*CHECK_LOG_ABORT(tid, status);*/                                            \
  /*if (status == _XABORT_CONFLICT) printf("CONFLICT: [start=%i, end=%i]\n", \ \
  NH_global_logs[TM_tid_var]->start, NH_global_logs[TM_tid_var]->end); */

#undef NH_before_write
#define NH_before_write(addr, val) ({   \
  LOG_nb_writes++;                      \
  LOG_push_addr(TM_tid_var, addr, val); \
})

#undef NH_write
#define NH_write(addr, val) ({                                                    \
  /*assert((intptr_t)al_pool <= (intptr_t)addr && (intptr_t)addr <= (intptr_t)al_pool + al_sz);*/\
  GRANULE_TYPE buf = val;                                                         \
  NH_before_write(addr, val);                                                     \
  memcpy(addr, &(buf), sizeof(GRANULE_TYPE)); /* *((GRANULE_TYPE*)addr) = val; */ \
  NH_after_write(addr, val);                                                      \
  val;                                                                            \
})

#undef NH_alloc
#define NH_alloc(fn, size) ({    \
  al_fn = fn;                    \
  al_sz = size;                  \
  al_pool = ALLOC_MEM(fn, size); \
  al_pool;                       \
})
extern size_t al_sz;
extern char const *al_fn;
extern void *al_pool;

#undef NH_free
#define NH_free(pool) FREE_MEM(pool, al_sz)

#define REMAP_PRIVATE()                                                                       \
  {                                                                                           \
    int fd = open(al_fn, O_RDWR);                                                             \
    if (fd == -1)                                                                             \
      perror("open");                                                                         \
    void *tmp = mmap(al_pool, al_sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, 0); \
    if (tmp == MAP_FAILED)                                                                    \
      perror("mmap");                                                                         \
    close(fd);                                                                                \
  }

// TODO: comment for testing with STAMP
/* #ifndef USE_MALLOC
  #if DO_CHECKPOINT == 5
  #undef  NH_alloc
  #undef  NH_free
  #define NH_alloc(size) malloc(size)
  #define NH_free(pool)  free(pool)
  #else
  #undef  NH_alloc
  #undef  NH_free
  #define NH_alloc(size) NVHTM_malloc(size)
  #define NH_free(pool)  NVHTM_free(pool)
  #endif
  #endif */

#ifdef __cplusplus
}
#endif

#endif /* NH_SOL_H */
