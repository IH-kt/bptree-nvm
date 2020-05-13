#include "extra_globals.h"

// ####################################################
// ### LOG VARIABLES ##################################
// ####################################################
// global

CL_ALIGN NVLog_s **NH_global_logs;
void* LOG_global_ptr;
int is_sigsegv = 0;
// thread local
__thread CL_ALIGN NVLog_s *nvm_htm_local_log;
__thread CL_ALIGN int LOG_nb_wraps;
__thread CL_ALIGN NVLogLocal_s LOG_local_state;
#ifdef STAT
__thread double abort_time_thread = 0;
double abort_time_all = 0;
__thread struct timespec transaction_start;
__thread struct timespec transaction_abort_end;
unsigned int *checkpoint_by_flags;
unsigned int checkpoint_by[3];
double checkpoint_section_time[4];
int *checkpoint_empty;
#ifdef WRITE_AMOUNT_NVHTM
unsigned long no_filter_write_amount;
unsigned long filtered_write_amount;
#endif
#endif
#ifdef PARALLEL_CHECKPOINT
int number_of_checkpoint_threads = 1;
sem_t cp_back_sem;
sem_t cpthread_finish_sem;
checkpoint_args_s *cp_thread_args;
#ifdef STAT
double *parallel_checkpoint_section_time_thread[2];
#endif
#ifdef LOG_COMPRESSION
CL_ALIGN NVLog_s **NH_global_compressed_logs;
void* LOG_compressed_global_ptr;
#endif
#endif
// ####################################################
