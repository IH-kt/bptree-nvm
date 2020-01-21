ifeq ($(type), nvhtm)
NVHTM_DIR       := $(ROOT_DIR)/nvhtm
NVM_DEP_DIR     := $(NVHTM_DIR)/DEPENDENCIES
HTM_ALG_DIR     := $(NVM_DEP_DIR)/htm_alg
NVHTM_SC_DIR    := $(ROOT_DIR)/nvhtm-selfcontained
MIN_NVM_DIR     := $(NVHTM_SC_DIR)/nvm-emulation

NVHTM_INCLUDES  := -I$(NVHTM_DIR)/common/ -I$(MIN_NVM_DIR)/include/\
                   -I$(NVM_DEP_DIR)/arch_dep/include/ -I$(HTM_ALG_DIR)/include/\
                   -I$(NVHTM_DIR)/nvhtm_common/ -I$(NVHTM_DIR)/nvhtm_pc/
NVHTM_DEFINES   := -DDO_CHECKPOINT=5 -DSOLUTION=4 -DSORT_ALG=5
NVHTM_LIBS      := -L$(MIN_NVM_DIR)/bin -L$(HTM_ALG_DIR)/bin -lhtm_sgl -lminimal_nvm
NVHTM_MAKE_ARGS := SOLUTION=4 DO_CHECKPOINT=5 USE_MIN_NVM=0
ifeq ($(forward), 1)
    NVHTM_MAKE_ARGS += SORT_ALG=4
else
    NVHTM_MAKE_ARGS += SORT_ALG=5
endif

ifeq ($(stats),1)
    MIN_NVM_FLAG    := "-DSTAT"
    NVHTM_MAKE_ARGS += STAT=1
	NVHTM_DEFINES	+= -DSTAT
    HTM_SGL_FLAG    := "-DCMAKE_CXX_FLAGS=-DSTAT -DCMAKE_C_FLAGS=-DSTAT"
else
    MIN_NVM_FLAG    :=
endif
endif

NVHTM_CFLAGS    := $(NVHTM_DEFINES) $(NVHTM_INCLUDES) $(PMDK_INCLUDES) -lpthread -mrtm $(NVHTM_LIBS) $(PMDK_LIBS)

ifeq ($(db), 1)
    NVHTM_MOD_DIR   := $(ROOT_DIR)/nvhtm_modification_files_double_buffering
else
    ifeq ($(chkpa),1)
        NVHTM_MOD_DIR   := $(ROOT_DIR)/nvhtm_modification_files_checkpoint_all
    else
        NVHTM_MOD_DIR   := $(ROOT_DIR)/nvhtm_modification_files_plain
    endif
endif
