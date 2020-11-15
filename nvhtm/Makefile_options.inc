# TODO: optional flags defines
OPT ?= -O0
USE_P8 ?= 0
USE_MIN_NVM ?= 1
LOG_SIZE ?= 10000
# percentage of the log to free-up
THRESHOLD ?= 0.0
# sleep time of the log manager (nano-seconds)
PERIOD ?= 10
ROOT ?= ./

FILTER ?= 0.50
BUDGET ?= 20
IS_BATCH ?= 0

DEFINES += -DNVMHTM_LOG_SIZE=$(LOG_SIZE) \
    -DSORT_ALG=$(SORT_ALG) \
    -DLOG_FILTER_THRESHOLD=$(FILTER) \
    -DHTM_SGL_INIT_BUDGET=$(BUDGET)

####
DO_CHECKPOINT ?= 0
####
# Available checkpoints:
#  1 - periodic
#  2 - reactive
#  3 - no_manager
#  4 - wrap
#  5 - fork
####

####
SOLUTION ?= 3
####
# Available solutions:
#  1 - HTM
#  2 - PHTM
#  3 - NVHTM Logical Clock
#  4 - NVHTM Physical Clock
####

####
SORT_ALG ?= 4
####
# Available solutions:
#  1 - Using a map/set to sort the logs
#  2 - Array
#  3 - No sort - apply next
#  4 - 2 threads: sort + apply
#  5 - Backward approach
####

ifeq ($(NDEBUG),1)
DEFINES += -DNDEBUG=1
endif

USE_MALLOC ?= 0

ifeq ($(USE_MALLOC),1)
DEFINES  += -DUSE_MALLOC
endif

ifeq ($(SOLUTION),1)
include $(ROOT)/Makefile_htm_only.inc
DEFINES  += -DHTM_ONLY
endif

ifeq ($(SOLUTION),2)
include $(ROOT)/Makefile_phtm.inc
DEFINES  += -DAVNI_SOL
endif

FLAG_CHECKPOINT :=

ifneq ($(DO_CHECKPOINT),0)
FLAG_CHECKPOINT += -DDO_CHECKPOINT=$(DO_CHECKPOINT)
FLAG_CHECKPOINT += -DLOG_THRESHOLD=$(THRESHOLD)
FLAG_CHECKPOINT += -DLOG_PERIOD=$(PERIOD)
endif

ifeq ($(SOLUTION),3)
include $(ROOT)/Makefile_nvhtm_lc.inc
DEFINES  += -DREDO_COUNTER -DVALIDATION=2 $(FLAG_CHECKPOINT)
endif

ifeq ($(SOLUTION),4)
include $(ROOT)/Makefile_nvhtm_pc.inc
DEFINES  += -DREDO_TS -DVALIDATION=3 $(FLAG_CHECKPOINT)
endif

ifeq ($(USE_P8),1)
# __powerpc__ already defined. Drop USE_P8
DEFINES  += -DUSE_P8 -mhtm
else
DEFINES  += -mrtm
endif

ifeq ($(USE_MIN_NVM),1)
# the other one is bugged
DEFINES  += -DUSE_MIN_NVM
endif

GCC_MAJOR:=$(shell gcc -dumpversion | cut -d'.' -f1)

ifeq ($(GCC_MAJOR),4)
DEFINES += -DOLD_ALLOC
endif

ifeq ($(IS_BATCH),1)
DEFINES += -DAPPLY_BATCH_TX
endif

ifeq ($(STAT), 1)
DEFINES += -DSTAT
endif

ifeq ($(NSTAT), 1)
DEFINES += -DNSTAT
endif

ifeq ($(REDUCE_CP), 1)
DEFINES += -DREDUCE_CP
endif

ifeq ($(FAW_CHECKPOINT), 1)
DEFINES += -DFAW_CHECKPOINT
endif

ifeq ($(USE_PMEM), 1)
DEFINES += -DUSE_PMEM
endif

ifeq ($(NO_EMPTY_LOOP_TIME), 1)
DEFINES += -DNO_EMPTY_LOOP_TIME
endif

ifeq ($(WRITE_AMOUNT_NVHTM), 1)
DEFINES += -DWRITE_AMOUNT_NVHTM
endif

ifeq ($(PARALLEL_CHECKPOINT), 1)
DEFINES += -DPARALLEL_CHECKPOINT
endif

ifeq ($(LOG_COMPRESSION), 1)
DEFINES += -DLOG_COMPRESSION
endif

ifeq ($(MEASURE_PART_CP), 1)
DEFINES += -DMEASURE_PART_CP
endif

ifeq ($(CHECK_TASK_DISTRIBUTION), 1)
DEFINES += -DCHECK_TASK_DISTRIBUTION
endif

ifeq ($(LARGE_DIV), 1)
DEFINES += -DLARGE_DIV
endif

ifeq ($(NUMBER_OF_ENTRIES), 1)
DEFINES += -DNUMBER_OF_ENTRIES
endif

ifeq ($(OPT_COMMIT), 1)
DEFINES += -DOPT_COMMIT
endif

ifeq ($(NO_FENCE), 1)
DEFINES += -DNO_FENCE
endif

ifeq ($(NO_FLUSH), 1)
DEFINES += -DNO_FLUSH
endif

ifeq ($(NO_CHECKPOINTER), 1)
DEFINES += -DNO_CHECKPOINTER
endif

ifeq ($(DISABLE_VALIDATION), 1)
DEFINES += -DDISABLE_VALIDATION
endif

ifeq ($(NO_COMMIT_TIME), 1)
DEFINES += -DNO_COMMIT_TIME
endif

ifeq ($(CP_NOTHING), 1)
DEFINES += -DCP_NOTHING
endif

ifeq ($(CP_INITIALIZE_ONLY), 1)
DEFINES += -DCP_INITIALIZE_ONLY
endif

ifeq ($(CP_NOWRITE), 1)
DEFINES += -DCP_NOWRITE
endif

ifeq ($(CP_NOFLUSH), 1)
DEFINES += -DCP_NOFLUSH
endif

ifeq ($(NUMBER_OF_WAIT_TIME), 1)
DEFINES += -DNUMBER_OF_WAIT_TIME
endif

ifeq ($(MAX_TX_SIZE), 1)
DEFINES += -DMAX_TX_SIZE
endif

ifeq ($(LARGE_PLOG), 1)
DEFINES += -DLARGE_PLOG
endif

ifeq ($(FUNCTIONIZE_AT), 1)
DEFINES += -DFUNCTIONIZE_AT
endif

ifeq ($(TX_SIZE), 1)
DEFINES += -DTX_SIZE
endif
