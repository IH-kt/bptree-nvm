NVHTM_DIR=$(ROOT_DIR)/nvhtm-selfcontained/nv-htm
NVM_DEP_DIR=$(ROOT_DIR)/nvhtm-selfcontained
NVHTM_INCLUDES=-I$(NVHTM_DIR)/common/ -I$(NVM_DEP_DIR)/nvm-emulation/include/ -I$(NVM_DEP_DIR)/arch-dep/include/ -I$(NVM_DEP_DIR)/htm-alg/include/ -I$(NVHTM_DIR)/nvhtm_common/ -I$(NVHTM_DIR)/nvhtm_pc/
NVHTM_DEFINES=-DCPU_MAX_FREQ=3900000 -DMAX_PHYS_THRS=18 -DNVMHTM_LOG_SIZE=1000 -DSORT_ALG=4 -DLOG_FILTER_THRESHOLD=0.50 -DHTM_SGL_INIT_BUDGET=20 -DREDO_TS -DVALIDATION=3  -DDO_CHECKPOINT=5 -DLOG_THRESHOLD=0.0 -DLOG_PERIOD=10
NVHTM_LIBS=-L$(NVM_DEP_DIR)/nvm-emulation/bin -L$(NVM_DEP_DIR)/htm-alg/bin libnh.a -lhtm_sgl -lminimal_nvm

NVHTM_CFLAGS=$(NVHTM_DEFINES) $(NVHTM_INCLUDES) $(PMDK_INCLUDES) -lpthread -mrtm $(NVHTM_LIBS) $(PMDK_LIBS)
