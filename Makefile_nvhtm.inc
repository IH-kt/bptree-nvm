ifeq ($(type), nvhtm)
NVHTM_DIR		:= $(ROOT_DIR)/nvhtm
NVM_DEP_DIR		:= $(NVHTM_DIR)/DEPENDENCIES
HTM_ALG_DIR		:= $(NVM_DEP_DIR)/htm_alg
MIN_NVM_DIR		:= $(ROOT_DIR)/nvm-emulation

NVHTM_INCLUDES	:= -I$(NVHTM_DIR)/common/ -I$(MIN_NVM_DIR)/include/\
				   -I$(NVM_DEP_DIR)/arch_dep/include/ -I$(HTM_ALG_DIR)/include/\
				   -I$(NVHTM_DIR)/nvhtm_common/ -I$(NVHTM_DIR)/nvhtm_pc/
NVHTM_DEFINES	:= -DDO_CHECKPOINT=5 -DSOLUTION=4 -DSORT_ALG=5
NVHTM_LIBS		:= -L$(MIN_NVM_DIR)/bin -L$(HTM_ALG_DIR)/bin -lhtm_sgl -lminimal_nvm
NVHTM_MAKE_ARGS	:= SOLUTION=4 DO_CHECKPOINT=5 USE_MIN_NVM=0
ifeq ($(forward), 1)
	NVHTM_MAKE_ARGS += SORT_ALG=4
else
	NVHTM_MAKE_ARGS += SORT_ALG=5
endif
ifeq ($(reduce), 1)
	NVHTM_MAKE_ARGS += REDUCE_CP=1
	NVHTM_DEFINES   += -DREDUCE_CP
endif

ifeq ($(faw_cp), 1)
    NVHTM_MAKE_ARGS += FAW_CHECKPOINT=1
    NVHTM_DEFINES   += -DFAW_CHECKPOINT
endif

ifneq ($(emulator), 1)
    MIN_NVM_FLAG    += -DUSE_PMEM
    NVHTM_MAKE_ARGS += USE_PMEM=1
    NVHTM_DEFINES   += -DUSE_PMEM
endif

ifeq ($(stats),1)
	MIN_NVM_FLAG	+= -DSTAT
	NVHTM_MAKE_ARGS += STAT=1
	NVHTM_DEFINES	+= -DSTAT
	HTM_SGL_FLAG	+= -DCMAKE_CXX_FLAGS=-DSTAT -DCMAKE_C_FLAGS=-DSTAT
endif

ifeq ($(nstats),1)
	MIN_NVM_FLAG	+= -DNSTAT
	NVHTM_MAKE_ARGS += NSTAT=1
	NVHTM_DEFINES	+= -DNSTAT
	HTM_SGL_FLAG	+= -DCMAKE_CXX_FLAGS=-DNSTAT -DCMAKE_C_FLAGS=-DNSTAT
endif

ifeq ($(no_empty_loop),1)
    NVHTM_MAKE_ARGS += NO_EMPTY_LOOP_TIME=1
    NVHTM_DEFINES   += -DNO_EMPTY_LOOP_TIME
endif

ifeq ($(write_amount_nvhtm),1)
    NVHTM_MAKE_ARGS += WRITE_AMOUNT_NVHTM=1
    NVHTM_DEFINES   += -DWRITE_AMOUNT_NVHTM
    MIN_NVM_FLAG	+= -DWRITE_AMOUNT_NVHTM
endif

ifeq ($(parallel_cp),1)
    NVHTM_MAKE_ARGS += PARALLEL_CHECKPOINT=1
    NVHTM_DEFINES   += -DPARALLEL_CHECKPOINT
endif

ifeq ($(log_compression),1)
    NVHTM_MAKE_ARGS += LOG_COMPRESSION=1
    NVHTM_DEFINES   += -DLOG_COMPRESSION
endif

ifeq ($(measure_cp),1)
    NVHTM_MAKE_ARGS += MEASURE_PART_CP=1
    NVHTM_DEFINES   += -DMEASURE_PART_CP
endif

ifeq ($(task_dist),1)
    NVHTM_MAKE_ARGS += CHECK_TASK_DISTRIBUTION=1
    NVHTM_DEFINES   += -DCHECK_TASK_DISTRIBUTION
endif

ifeq ($(ldiv),1)
    NVHTM_MAKE_ARGS += LARGE_DIV=1
    NVHTM_DEFINES   += -DLARGE_DIV
endif

ifeq ($(entries),1)
    NVHTM_MAKE_ARGS += NUMBER_OF_ENTRIES=1
    NVHTM_DEFINES   += -DNUMBER_OF_ENTRIES
endif

ifeq ($(opt_c),1)
    NVHTM_MAKE_ARGS += OPT_COMMIT=1
    NVHTM_DEFINES   += -DOPT_COMMIT
endif

ifeq ($(no_fence),1)
    NVHTM_MAKE_ARGS += NO_FENCE=1
    NVHTM_DEFINES   += -DNO_FENCE
endif

ifeq ($(no_flush),1)
    NVHTM_MAKE_ARGS += NO_FLUSH=1
    NVHTM_DEFINES   += -DNO_FLUSH
endif

ifeq ($(no_cp),1)
    NVHTM_MAKE_ARGS += NO_CHECKPOINTER=1
    NVHTM_DEFINES   += -DNO_CHECKPOINTER
endif

ifeq ($(no_val),1)
    NVHTM_MAKE_ARGS += DISABLE_VALIDATION=1
    NVHTM_DEFINES   += -DDISABLE_VALIDATION
endif

ifeq ($(nctime),1)
    NVHTM_MAKE_ARGS += NO_COMMIT_TIME=1
    NVHTM_DEFINES   += -DNO_COMMIT_TIME
endif

ifeq ($(cpnt),1)
    NVHTM_MAKE_ARGS += CP_NOTHING=1
    NVHTM_DEFINES   += -DCP_NOTHING
endif

ifeq ($(cpit),1)
    NVHTM_MAKE_ARGS += CP_INITIALIZE_ONLY=1
    NVHTM_DEFINES   += -DCP_INITIALIZE_ONLY
endif

ifeq ($(cpnw),1)
    NVHTM_MAKE_ARGS += CP_NOWRITE=1
    NVHTM_DEFINES   += -DCP_NOWRITE
endif

ifeq ($(cpnf),1)
    NVHTM_MAKE_ARGS += CP_NOFLUSH=1
    NVHTM_DEFINES   += -DCP_NOFLUSH
endif

ifeq ($(nwt),1)
    NVHTM_MAKE_ARGS += NUMBER_OF_WAIT_TIME=1
    NVHTM_DEFINES   += -DNUMBER_OF_WAIT_TIME
endif

ifeq ($(max_tx_size),1)
    NVHTM_MAKE_ARGS += MAX_TX_SIZE=1
    NVHTM_DEFINES   += -DMAX_TX_SIZE
endif

ifeq ($(large_plog),1)
    NVHTM_MAKE_ARGS += LARGE_PLOG=1
    NVHTM_DEFINES   += -DLARGE_PLOG
endif

ifeq ($(func_at),1)
    NVHTM_MAKE_ARGS += FUNCTIONIZE_AT=1
    NVHTM_DEFINES   += -DFUNCTIONIZE_AT
endif

ifdef logsize
	LOGSZ	:= $(logsize)
else
	LOGSZ	:= 41943328
endif
NVHTM_MAKE_ARGS += LOG_SIZE=$(LOGSZ)

NVHTM_CFLAGS	:= $(NVHTM_DEFINES) $(NVHTM_INCLUDES) $(PMDK_INCLUDES) -lpthread -mrtm $(NVHTM_LIBS) $(PMDK_LIBS)

ifeq ($(db), 1)
	NVHTM_MOD_DIR   := $(ROOT_DIR)/nvhtm_modification_files_double_buffering
else
    NVHTM_MOD_DIR   := $(ROOT_DIR)/nvhtm_modification_files_plain
endif
endif
