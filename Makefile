include ./Makefile_location.inc
VPATH = $(BUILD_DIR):$(TEST_SRC_DIR)/$(TYPE):$(TEST_SRC_DIR)/$(TREE):$(BASE_BENCH_SRC_DIR):$(HTM_ALG_DIR)/bin:$(MIN_NVM_DIR)/bin

.PRECIOUS: %.o

PMDK_DIR		:= $(HOME)/local
PMDK_INCLUDES	:= -I$(PMDK_DIR)/include/
PMDK_LIBS		:= -L$(PMDK_DIR)/lib -lpmem

include ./Makefile_nvhtm.inc

ifdef no_persist
	NO_PERSIST := -DNPERSIST
else
	NO_PERSIST :=
endif
ifdef nclwb
	CLWB :=
else
	CLWB := -DCLWB
endif
ifeq ($(type), concurrent)
	CONCURRENT := -DCONCURRENT
else
	CONCURRENT :=
endif
ifeq ($(type), nvhtm)
	CONCURRENT := -DCONCURRENT
	NVHTM := -DNVHTM
	NVHTM_LIB := $(BUILD_DIR)/libnh.a
else
	NVHTM :=
endif
ifeq ($(time), time_part)
	TIME_PART := -DTIME_PART
else
	TIME_PART :=
endif
ifeq ($(tree), bptree)
	TREE_D		:= -DBPTREE
	TREE_OBJ	:= $(BPTREE_SRC:%.c=%.o)
else
	TREE_D		:=
	TREE_OBJ	:= $(FPTREE_SRC:%.c=%.o)
endif
ifeq ($(debug), 1)
	DEBUG := -DDEBUG
else
	DEBUG :=
endif
ifeq ($(cw), 1)
	CW	:= -DCOUNT_WRITE
else
	CW	:=
endif
ifeq ($(ca), 1)
	CA	:= -DCOUNT_ABORT
else
	CA	:=
endif
ifeq ($(fw), 1)
	FW		:= -DFREQ_WRITE
	DMN_DIR	:= $(ROOT_DIR)/dummy_min-nvm_wfreq
else
	FW	:=
	DMN_DIR	:= $(ROOT_DIR)/dummy_min-nvm
endif
ifeq ($(ts), 1)
	TS	:= -DTRANSACTION_SIZE
else
	TS	:=
endif

DEFINES = $(NVHTM) $(CLWB) $(CONCURRENT) $(NO_PERSIST) $(TIME_PART) $(TREE_D) $(DEBUG) $(CW) $(CA) $(FW) $(TS)

CC=gcc
CXX=g++
CFLAGS=-O0 -g -march=native -pthread $(DEFINES) -I$(INCLUDE_DIR) $(NVHTM_CFLAGS)

ALLOCATOR_OBJ=$(ALLOCATOR_SRC:%.c=%.o)
THREAD_MANAGER_OBJ=$(THREAD_MANAGER_SRC:%.c=%.o)

TEST_EXE		:= $(TEST_SRC_NAME:%.c=%.exe)
BASE_BENCH_EXE	:= $(BASE_BENCH_SRC_NAME:%.c=%.exe)
ALL_EXE			:= $(TEST_EXE) $(BASE_BENCH_EXE)

# make-test:
# 	echo $(TEST_SRC)
# 	echo $(TEST_EXE)

all: $(ALL_EXE)

%.exe:%.o $(TREE_OBJ) $(ALLOCATOR_OBJ) $(THREAD_MANAGER_OBJ) $(NVHTM_LIB)
	$(CXX) -o $(BUILD_DIR)/$@ $+ $(NVHTM_LIB) $(CFLAGS)

%.o:%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -o $@ $(CFLAGS) -c $+

$(NVHTM_LIB): libhtm_sgl.a libminimal_nvm.a
	cp -R $(ROOT_DIR)/nvhtm_modification_files/* $(NVHTM_DIR)
	make -C nvhtm clean
	make -C nvhtm $(NVHTM_MAKE_ARGS)
	mkdir -p $(BUILD_DIR)
	mv nvhtm/libnh.a $(NVHTM_LIB)

libhtm_sgl.a:
	cp -R $(ROOT_DIR)/nvhtm_modification_files/* $(NVHTM_DIR)
	(cd $(NVHTM_DIR)/DEPENDENCIES/htm_alg; ./compile.sh)

libminimal_nvm.a:
	cp -R $(DMN_DIR)/* $(MIN_NVM_DIR)
	(cd $(MIN_NVM_DIR); ./compile.sh)

test-make:
	echo $(NVHTM_CFLAGS)

clean:
	rm -f $(TREE_OBJ) $(ALLOCATOR_OBJ) $(THREAD_MANAGER_OBJ) $(ALL_EXE:%.exe=%.o)

dist-clean: clean
	rm -f $(addprefix $(BUILD_DIR)/, $(ALL_EXE)) $(NVHTM_LIB) $(MIN_NVM_DIR)/bin/libminimal_nvm.a $(NVHTM_DIR)/DEPENDENCIES/htm_alg/bin/libhtm_sgl.a
	(cd $(NVHTM_DIR); git checkout .; make clean)
	(cd $(NVHTM_SC_DIR); git checkout .)
