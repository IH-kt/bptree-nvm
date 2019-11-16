include ./Makefile_location.inc
VPATH = $(BUILD_DIR):$(TEST_SRC_DIR)/$(TYPE):$(BASE_BENCH_SRC_DIR)

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

DEFINES = $(NVHTM) $(CLWB) $(CONCURRENT) $(NO_PERSIST) $(TIME_PART)

CC=gcc
CXX=g++
CFLAGS=-O0 -g -march=native -pthread $(DEFINES) -I$(INCLUDE_DIR) $(NVHTM_CFLAGS)

FPTREE_OBJ=$(FPTREE_SRC:%.c=%.o)
ALLOCATOR_OBJ=$(ALLOCATOR_SRC:%.c=%.o)
THREAD_MANAGER_OBJ=$(THREAD_MANAGER_SRC:%.c=%.o)

TEST_EXE		:= $(TEST_SRC_NAME:%.c=%.exe)
BASE_BENCH_EXE	:= $(BASE_BENCH_SRC_NAME:%.c=%.exe)
ALL_EXE			:= $(TEST_EXE) $(BASE_BENCH_EXE)

all: $(ALL_EXE)

%.exe:%.o $(FPTREE_OBJ) $(ALLOCATOR_OBJ) $(THREAD_MANAGER_OBJ) $(NVHTM_LIB)
	$(CXX) -o $(BUILD_DIR)/$@ $+ $(CFLAGS)

%.o:%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $+ -o $@ $(CFLAGS)

libnh.a:
	./make_nv-htm_lib.sh
	mv libnh.a $(BUILD_DIR)

clean:

dist-clean: clean
	rm -f $(addprefix $(BUILD_DIR)/, $(ALL_EXE)) $(NVHTM_LIB)
