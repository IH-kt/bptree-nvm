#ifndef LOG_H
#define LOG_H

#include "extra_types.h"

#include "tm.h"
#include "nvhtm.h"
#include "nvhtm_helper.h"
#include "log_aux.h"
#include "log_forward.h"
#include "log_backward.h"
#include "utils.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
	#endif

	// TODO: the checkpointing is only done after commit, we must be sure
	// that the number of writes within transactions are never greater than:
	// NVMHTM_CHECKPOINT_CRITICAL * NVMHTM_LOG_SIZE

	// TODO: WAIT_DISTANCE must be smaller than size of the log minus max size TX
	#define WAIT_DISTANCE (512)

	// TODO: space it more, like 8 or 12
	#define WAIT_LOG_CONDITION \
	(LOG_local_state.counter > (LOG_local_state.size_of_log - 16))

	// this parameters are a bit random,
	// but they are meant to minimize HTM aborts
	#define TOO_FULL (LOG_local_state.size_of_log - WAIT_DISTANCE - 1)
	#define TOO_EMPTY 512

	// TODO: why was this?
	#define CHECK_AND_REQUEST(tid) ({ })

	#if DO_CHECKPOINT == 2
	// TODO: 0.05 and 0.5 are magic numbers

	// ocuppied ==> distance_ptr(log->start, log->end)

	// REACTIVE one
	#define FREE_LOG_SPACE(log) ({ \
		if (LOG_try_lock()) { \
			LOG_nb_wraps++; \
			while((LOG_local_state.size_of_log - distance_ptr(log->start, log->end)) < \
			LOG_local_state.size_of_log * LOG_THRESHOLD) { \
				NVHTM_reduce_logs(); \
				PAUSE(); \
			} \
			LOG_move_start_ptrs(); \
			LOG_unlock(); \
		} \
	})

	#define WAIT_MORE_LOG(log) ({ \
		if (distance_ptr(log->start, log->end) > (LOG_local_state.size_of_log - 2)) { \
			ts_s ts1_wait_log_time; \
			ts1_wait_log_time = rdtscp(); \
			while (distance_ptr(log->start, log->end) > (LOG_local_state.size_of_log - 2)) { \
				if (HTM_test()) HTM_named_abort(CODE_LOG_ABORT); \
				FREE_LOG_SPACE(log); \
				PAUSE(); \
			} \
			LOG_before_TX(); \
			NH_count_blocks++; \
			NH_time_blocked += rdtscp() - ts1_wait_log_time; \
		} \
	})

	#define CHECK_LOG_ABORT(TM_tid_var, TM_status_var) ({ \
		if (HTM_is_named(TM_status_var) == CODE_LOG_ABORT) { \
			NVLog_s *log = LOG_get(TM_tid_var); \
			FREE_LOG_SPACE(log); \
			LOG_before_TX(); \
		} \
	})

	#elif DO_CHECKPOINT == 3 /* THIS IS NO LONGER USED ---> freze on full */
	#define WAIT_MORE_LOG(log) ({ \
		if (WAIT_LOG_CONDITION) { \
			ts_s ts1_wait_log_time; \
			ts1_wait_log_time = rdtscp(); \
			if (distance_ptr(LOG_local_state.start, log->end) > (LOG_local_state.size_of_log - 2)) { \
				if (HTM_test()) HTM_named_abort(CODE_LOG_ABORT); \
				LOG_nb_wraps++; \
			} \
			while (distance_ptr(log->start, log->end) > (LOG_local_state.size_of_log - 2)) PAUSE(); \
			LOG_before_TX(); \
			NH_count_blocks++; \
			NH_time_blocked += rdtscp() - ts1_wait_log_time; \
		} \
	})

	#define CHECK_LOG_ABORT(TM_tid_var, TM_status_var) /* empty */

	#elif DO_CHECKPOINT == 5 || DO_CHECKPOINT == 1 /* FORK || PERIODIC */
#define WAIT_MORE_LOG(log) ({                                                                  \
	if (WAIT_LOG_CONDITION)                                                                    \
	{                                                                                          \
		ts_s ts1_wait_log_time, ts2_wait_log_time;                                             \
		if (HTM_test()) {                                                                        \
			HTM_named_abort(CODE_LOG_ABORT);                                                   \
        } else if (HTM_SGL_var)\
        {\
            fprintf(stderr, "wait_more_log:%d\n", TM_tid_var);\
            assert(0); /* stopped during lock execution */                                         \
        } else {\
            fprintf(stderr, "???\n");\
        }\
		/*ts1_wait_log_time = rdtscp();                                                          \
		while (distance_ptr(log->start, log->end) >                                            \
			   (LOG_local_state.size_of_log - 32))                                             \
		{                                                                                      \
			if (((*NH_checkpointer_state) & 0x1) == 0)                                         \
				sem_post(NH_chkp_sem);                                                         \
			PAUSE();                                                                           \
		}                                                                                      \
		NH_count_blocks++;                                                                     \
		LOG_before_TX();                                                                       \
		ts2_wait_log_time = rdtscp();                                                          \
		NH_time_blocked += rdtscp() - ts1_wait_log_time;*/                                       \
		/*double lat = (double)(ts2_wait_log_time - ts1_wait_log_time) / (double)CPU_MAX_FREQ; \
		 if (lat > 100) printf("Blocked for %f ms\n", lat); */                                 \
	}                                                                                          \
})

	// TODO: there are aborts marked as EXPLICIT when the log is empty, WHY?
#define CHECK_LOG_ABORT(tid, TM_status_var) \
	if (HTM_is_named(TM_status_var) == CODE_LOG_ABORT) { \
		ts_s ts1_wait_log_time, ts2_wait_log_time; \
		ts1_wait_log_time = rdtscp(); \
		NVLog_s *log = NH_global_logs[TM_tid_var]; \
		/*if ((LOG_local_state.counter == distance_ptr(log->start, log->end) \
			&& (LOG_local_state.size_of_log - LOG_local_state.counter) < WAIT_DISTANCE) \
			|| (distance_ptr(log->end, log->start) < WAIT_DISTANCE \
			&& log->end != log->start)) \
                printf("check log abort:%d\n", tid);*/\
        /*printf("%d: check_log_abort -> size = %d, counter = %d, local start = %d, local end = %d, global start = %d, global end = %d, log_at_start = %p, current_log = %p\n", TM_tid_var, LOG_local_state.size_of_log, LOG_local_state.counter, LOG_local_state.start, LOG_local_state.end, log->start, log->end, log_at_tx_start, NH_global_logs);*/\
        persistent_checkpointing[TM_tid_var].flag = 0;\
        int sem_val;\
        sem_getvalue(NH_chkp_sem, &sem_val);\
		while ((LOG_local_state.counter == distance_ptr(log->start, log->end) \
			&& (LOG_local_state.size_of_log - LOG_local_state.counter) < WAIT_DISTANCE) \
			|| (distance_ptr(log->end, log->start) < WAIT_DISTANCE \
			&& log->end != log->start)) { \
                sem_getvalue(NH_chkp_sem, &sem_val);\
				if (*NH_checkpointer_state == 0 && sem_val <= 0) {\
                    sem_post(NH_chkp_sem); \
                }\
				PAUSE(); \
                log = NH_global_logs[TM_tid_var]; \
                /*printf("check_log_abort\n");*/\
		} \
		NH_count_blocks++; \
		nvm_htm_local_log = NH_global_logs[TM_tid_var];\
		LOG_before_TX(); \
        /*printf("cla %d-%d: start = %d, end = %d, local start = %d, local end = %d\n", tid, TM_tid_var, NH_global_logs[tid]->start, NH_global_logs[tid]->end, LOG_local_state.start, LOG_local_state.end);*/\
		ts2_wait_log_time = rdtscp(); \
		NH_time_blocked += ts2_wait_log_time - ts1_wait_log_time; \
		/* double lat = (double)(ts2_wait_log_time - ts1_wait_log_time) / (double)CPU_MAX_FREQ; \
		if (lat > 100) printf("Blocked for %f ms\n", lat); */ \
	} else {\
		nvm_htm_local_log = NH_global_logs[TM_tid_var];\
        LOG_local_state.start = nvm_htm_local_log->start;                                                  \
        LOG_local_state.end = nvm_htm_local_log->end;                                                      \
        LOG_local_state.counter = distance_ptr((int)LOG_local_state.start,                   \
                                               (int)LOG_local_state.end);                    \
    }

	#else /* DEFAULT: WRAP */
	#define WAIT_MORE_LOG(log) ({ \
		if (WAIT_LOG_CONDITION) { \
			ts_s ts1_wait_log_time; \
			ts1_wait_log_time = rdtscp(); \
			if (distance_ptr(log->start, log->end) > (LOG_local_state.size_of_log - 2)) { \
				LOG_nb_wraps++; \
				log->start_tx = -1; \
				log->start = 0; \
				log->end = 0; /* resets */ \
			} \
			LOG_local_state.counter = 0; \
			NH_time_blocked += rdtscp() - ts1_wait_log_time; \
		} \
	})

	#define CHECK_LOG_ABORT(TM_tid_var, TM_status_var) /* empty */

	#endif /* DO_CHECKPOINT */

    void set_log_file_name(char const *);
	void LOG_init(int nb_threads, int fresh);
	void LOG_alloc(int tid, const char *pool_file, int fresh);
	void LOG_thr_init(int tid);
	void LOG_clear(int tid);
	int LOG_has_new_writes(int tid);
	NVLog_s *LOG_get(int tid);

	void LOG_attach_shared_mem();

#define LOG_push_entry(log, entry) ({                                       \
	int end = LOG_local_state.end /* log->end */, new_end;                  \
	LOG_local_state.counter++;                                              \
	new_end = ptr_mod_log(end, 1);                                          \
	/*log->ptr[end].addr = entry.addr; log->ptr[end].value = entry.value;*/ \
	MN_count_writes++;                                                      \
	memcpy(&(log->ptr[end]), &entry, sizeof(NVLogEntry_s));                 \
	/*TODO: this aborts the transactions: */                                \
	/*MN_write(&(log->ptr[end]), &(entry), sizeof(NVLogEntry_s), 0);*/      \
	/* log->end */ LOG_local_state.end = new_end;                           \
	assert(new_end < LOG_local_state.size_of_log);                          \
	new_end;                                                                \
})

// this is called inside of the transaction
// void LOG_push_addr(int tid, GRANULE_TYPE *addr, GRANULE_TYPE value);
#define LOG_push_addr(_tid, adr, val) ({                                 \
	int id = TM_tid_var;                                                \
	NVLog_s *log = _tid == id ? nvm_htm_local_log : NH_global_logs[id]; \
	WAIT_MORE_LOG(log); /* only waits on the addr */                    \
	int end = LOG_local_state.end /* log->end */, new_end;              \
	NVLogEntry_s entry;                                                 \
	entry.addr = (GRANULE_TYPE *)adr;                                   \
	entry.value = (GRANULE_TYPE)val;                                    \
	new_end = LOG_push_entry(log, entry);                               \
    /*printf("%d: new_end = %d\n", _tid, new_end);*/\
    new_end;\
})

// void LOG_push_ts(int tid, ts_s ts);
#define LOG_push_ts(_tid, ts) ({                                         \
	int id = TM_tid_var;                                                \
	NVLog_s *log = _tid == id ? nvm_htm_local_log : NH_global_logs[id]; \
	int end = LOG_local_state.end, new_end;                             \
	NVLogEntry_s entry;                                                 \
	entry.addr = (GRANULE_TYPE *)LOG_TS;                                \
	entry.value = ts;                                                   \
	new_end = LOG_push_entry(log, entry);                               \
	log->start_tx = new_end;                                            \
	log->end_last_tx = end;                                             \
})

	void LOG_push_malloc(int tid, GRANULE_TYPE *addr);

	// also prefetch a bit
#define LOG_before_TX() ({                                                               \
    int id = TM_tid_var;\
	NVLog_s *log = NH_global_logs[id];                                           \
	LOG_nb_writes = 0;                                                                   \
	LOG_local_state.start = log->start;                                                  \
	LOG_local_state.end = log->end;                                                      \
	LOG_local_state.counter = distance_ptr((int)LOG_local_state.start,                   \
										   (int)LOG_local_state.end);                    \
	if (LOG_local_state.counter >= APPLY_BACKWARD_VAL)                                   \
	{                                                                                    \
        int sem_val;\
        sem_getvalue(NH_chkp_sem, &sem_val);\
		if (*NH_checkpointer_state == 0 && sem_val <= 0)                                       \
		{                                                                                \
            /*printf("%d: LOG_before_TX -> counter = %d, start = %d, end = %d\n", id, LOG_local_state.counter, LOG_local_state.start, LOG_local_state.end);*/\
			sem_post(NH_chkp_sem);                                                       \
			PAUSE();                                                                     \
		}                                                                                \
	}                                                                                    \
	/* prefetch */                                                                       \
	if (LOG_local_state.counter < LOG_local_state.size_of_log - 16)                      \
	{                                                                                    \
		NH_global_logs[id]->ptr[ptr_mod_log(LOG_local_state.end, 1)].value = 0;  \
		NH_global_logs[id]->ptr[ptr_mod_log(LOG_local_state.end, 3)].value = 0;  \
	}                                                                                    \
	if (LOG_local_state.counter < LOG_local_state.size_of_log - 128)                     \
	{                                                                                    \
		NH_global_logs[id]->ptr[ptr_mod_log(LOG_local_state.end, 8)].value = 0;  \
		NH_global_logs[id]->ptr[ptr_mod_log(LOG_local_state.end, 16)].value = 0; \
		NH_global_logs[id]->ptr[ptr_mod_log(LOG_local_state.end, 24)].value = 0; \
		NH_global_logs[id]->ptr[ptr_mod_log(LOG_local_state.end, 32)].value = 0; \
		NH_global_logs[id]->ptr[ptr_mod_log(LOG_local_state.end, 64)].value = 0; \
	}                                                                                    \
    /*printf("before %d:log_start = %d, log_end = %d, log_local_start = %d, log_local_end = %d\n",\
            id, NH_global_logs[id]->start, NH_global_logs[id]->end, LOG_local_state.start, LOG_local_state.end);*/\
})

#if DO_CHECKPOINT == 4
#define LOG_after_TX() ({                         \
	NVLog_s *log = NH_global_logs[TM_tid_var];    \
	MN_write(&(log->end), &(LOG_local_state.end), \
			 sizeof(LOG_local_state.end), 0);     \
	MN_count_spins++;                             \
	/*log->end = LOG_local_state.end;*/           \
	__sync_synchronize();                         \
})
#else
#define LOG_after_TX() ({                                \
    int tid = TM_tid_var;\
	NVLog_s *log = NH_global_logs[tid];              \
	int log_end, log_start;                                 \
	log_end = log->end;                                     \
	log_start = log->start;                                 \
    /*printf("after %d-%d:log_start = %d, log_end = %d, log_local_start = %d, log_local_end = %d\n",\
            tid, HTM_SGL_vars.tid, log_start, log_end, LOG_local_state.start, LOG_local_state.end);*/\
	while (!(distance_ptr(log_start, log_end) <=            \
			 distance_ptr(log_start, LOG_local_state.end))) {\
		log_start = log->start;                             \
             fprintf(stderr, "LOG_after_TX:%d: 1 -> %d, 2 -> %d\n", tid, _NH_global_logs1[tid]->start, _NH_global_logs2[tid]->start);\
             assert(0);\
        }\
	MN_write(&(log->end), &(LOG_local_state.end),           \
			 sizeof(LOG_local_state.end), 0);               \
	/* MN_count_spins++; */                                 \
	/* log->end = LOG_local_state.end; */                   \
	__sync_synchronize();                                   \
})
#endif

#define LOG_count_writes(tid) ({ LOG_nb_writes; })

	void LOG_get_ts_before_tx(int tid);

	// the two below update start_ptr and not start (to avoid contention)
	int LOG_checkpoint_apply_one(); // map
	int LOG_checkpoint_apply_one2(); // array
	int LOG_checkpoint_apply_one3(); // next (not ACID)
	void LOG_checkpoint_apply_N(int n);
	void LOG_checkpoint_apply_N_update_after(int n);
	void LOG_fake_advance_ptrs(int n);

	// This one moves start to start_ptr (may cause some contention)
	void LOG_move_start_ptrs();
	void LOG_handle_checkpoint();
    void NH_start_freq();
    void NH_reset_nb_cp();

	#define ptr_mod_log(ptr, inc) ({ \
		LOG_MOD2((long long)ptr + (long long)inc, LOG_local_state.size_of_log); \
	})

    typedef union pschkp_t {
        int flag;
        char pad[CACHE_LINE_SIZE];
    } pschkp_t;
extern pschkp_t *persistent_checkpointing;

/*({ \
		int res; \
		res = ptr_mod((int)ptr, (int)inc, LOG_local_state.size_of_log); \
		assert(inc != 1 || ptr == (LOG_local_state.size_of_log-1) \
		|| (res == ptr+1)); \
		res; \
})*/

	#ifdef __cplusplus
}
#endif

#endif /* LOG_H */
