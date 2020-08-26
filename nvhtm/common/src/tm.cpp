#include "tm.h"

#include "htm_retry_template.h"

#include <cstdlib>
#include <thread>
#include <mutex>

#ifndef TM_INIT_BUDGET
#define TM_INIT_BUDGET 5
#endif

using namespace std;

static CL_ALIGN mutex mtx;
static CL_ALIGN int count_serl_txs;
static CL_ALIGN int init_budget = TM_INIT_BUDGET;
static CL_ALIGN int threads;

static CL_ALIGN int *is_record;
static CL_ALIGN int **errors;

void TM_init_nb_threads(int nb_threads)
{
    int i;
    threads = nb_threads;

    // if malloc it may not get aligned
    //    TM_SGL_var = (int*) aligned_alloc(CACHE_LINE_SIZE, CACHE_LINE_SIZE);
    TM_SGL_var = false;

    is_record = (int*) malloc(sizeof (int) * nb_threads);
    errors = (int**) malloc(sizeof (int*) * nb_threads);
    for (i = 0; i < nb_threads; ++i) {
        errors[i] = (int*) malloc(sizeof (int) * HTM_NB_ERRORS);
        memset(errors[i], 0, sizeof (int) * HTM_NB_ERRORS);
    }

    // TODO: does not work in older systems
    ALLOC_FN(htm_tx_val_counters, tx_counters_s,
             sizeof (tx_counters_s) * nb_threads);
    //    htm_tx_val_counters = (tx_counters_s*)
    //            aligned_alloc(CACHE_LINE_SIZE, sizeof (tx_counters_s) * nb_threads);
    memset(htm_tx_val_counters, 0, sizeof (tx_counters_s) * nb_threads);

    //    for (i = 0; i < nb_threads; ++i) {
    //        htm_tx_val_counters[i*DIST_CL] = (tx_counters_s*)
    //                aligned_alloc(CACHE_LINE_SIZE, sizeof (tx_counters_s));
    //        memset(htm_tx_val_counters[i*DIST_CL], 0, sizeof (tx_counters_s));
    //    }
}

void TM_set_is_record(int tid, int is_rec)
{
#ifndef NSTAT
    HTM_set_is_record(is_rec); // TODO: this is thread-local
    is_record[tid] = is_rec;
#endif
}

void TM_inc_fallback(int tid)
{
#ifndef NSTAT
    if (is_record[tid]) {
        errors[tid][HTM_FALLBACK]++;
    }
#endif
}

void TM_inc_error(int tid, HTM_STATUS_TYPE error)
{
#ifndef NSTAT
    if (is_record[tid] || error == HTM_CODE_SUCCESS) {
        HTM_ERROR_INC(error, errors[tid]);
    }
#endif
}

void TM_reset_error()
{
#ifdef STAT
    int i, error;
    fprintf(stderr, "reset error counter\n");
    for (i = 0; i < threads; ++i) {
        fprintf(stderr, "thread %d:\n", i);
        fprintf(stderr, "\tSUCCESS  = %d\n", errors[i][HTM_SUCCESS]);
        fprintf(stderr, "\tABORT    = %d\n", errors[i][HTM_ABORT]);
        fprintf(stderr, "\tEXPLICIT = %d\n", errors[i][HTM_EXPLICIT]);
        fprintf(stderr, "\tRETRY    = %d\n", errors[i][HTM_RETRY]);
        fprintf(stderr, "\tCONFLICT = %d\n", errors[i][HTM_CONFLICT]);
        fprintf(stderr, "\tCAPACITY = %d\n", errors[i][HTM_CAPACITY]);
        fprintf(stderr, "\tDEBUG    = %d\n", errors[i][HTM_DEBUG]);
        fprintf(stderr, "\tNESTED   = %d\n", errors[i][HTM_NESTED]);
        fprintf(stderr, "\tOTHER    = %d\n", errors[i][HTM_OTHER]);
        fprintf(stderr, "\tFALLBACK = %d\n", errors[i][HTM_FALLBACK]);
        for (error = 0; error < HTM_NB_ERRORS; error++) {
            errors[i][error] = 0;
        }

    }
#endif
}

int TM_get_error_count(int error)
{
    int i, res = 0;
#ifndef NSTAT
    for (i = 0; i < threads; ++i) {
        res += errors[i][error];
    }
#endif
    return res;
}

int TM_inc_global_counter(int tid)
{
    htm_tx_val_counters[tid].global_counter = ++LOG_global_counter;
    return htm_tx_val_counters[tid].global_counter;
}

void TM_inc_local_counter(int tid)
{
    ++(htm_tx_val_counters[tid].local_counter);
}

int TM_get_local_counter(int tid)
{
    return htm_tx_val_counters[tid].local_counter;
}

int TM_get_global_counter(int tid)
{
    return htm_tx_val_counters[tid].global_counter;
}

int TM_get_nb_threads()
{
    return threads;
}
