#ifndef EXTRA_GLOBALS_H
#define EXTRA_GLOBALS_H

#include "extra_types.h"

// ####################################################
// ### LOG VARIABLES ##################################
// ####################################################
// global
#define NH_global_logs (*nh_glog_ref)
extern CL_ALIGN NVLog_s ***nh_glog_ref;
extern CL_ALIGN NVLog_s **_NH_global_logs1;
extern CL_ALIGN NVLog_s **_NH_global_logs2;
extern void* LOG_global_ptr;
// thread local
extern __thread CL_ALIGN NVLog_s *nvm_htm_local_log;
extern __thread CL_ALIGN int LOG_nb_wraps;
extern __thread CL_ALIGN NVLogLocal_s LOG_local_state;
// ####################################################

#endif /* EXTRA_GLOBALS_H */
