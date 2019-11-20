ifeq ($(type), nvhtm)
NVHTM_DIR       := $(ROOT_DIR)/nvhtm
NVM_DEP_DIR     := $(ROOT_DIR)/nvhtm/DEPENDENCIES
HTM_ALG_DIR     := $(ROOT_DIR)/nvhtm/DEPENDENCIES/htm_alg
MIN_NVM_DIR     := $(ROOT_DIR)/nvhtm-selfcontained/nvm-emulation

NVHTM_INCLUDES  := -I$(NVHTM_DIR)/common/ -I$(MIN_NVM_DIR)/include/\
                   -I$(NVM_DEP_DIR)/arch_dep/include/ -I$(HTM_ALG_DIR)/include/\
                   -I$(NVHTM_DIR)/nvhtm_common/ -I$(NVHTM_DIR)/nvhtm_pc/
NVHTM_DEFINES   := -DDO_CHECKPOINT=5 -DSOLUTION=4 -DSORT_ALG=5
NVHTM_LIBS      := -L$(MIN_NVM_DIR)/bin -L$(HTM_ALG_DIR)/bin -lhtm_sgl -lminimal_nvm
NVHTM_CFLAGS    := $(NVHTM_DEFINES) $(NVHTM_INCLUDES) $(PMDK_INCLUDES) -lpthread -mrtm $(NVHTM_LIBS) $(PMDK_LIBS)

NVHTM_MAKE_ARGS := SOLUTION=4 DO_CHECKPOINT=5 SORT_ALG=5
endif
