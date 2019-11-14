USE_PMDK=-DPMDK
# NO_PERSIST=-DNPERSIST
CLWB=-DCLWB
# CLWB=
CONCURRENT=-DCONCURRENT
# CONCURRENT=-UCONCURRENT
# ALLOCATOR=-DMT_ALLOCATOR

PMDK_DIR=$(HOME)/local
PMDK_INCLUDES=-I$(PMDK_DIR)/include/
PMDK_LIBS=-L$(PMDK_DIR)/lib -lpmem

include $(ROOT_DIR)/Makefile_nvhtm.inc

CC=gcc
CXX=g++
CFLAGS=-O0 -g -march=native -pthread $(NVHTM) $(CLWB) $(CONCURRENT) $(NO_PERSIST) $(ALLOCATOR) $(TIME_PART) -I$(ROOT_DIR)/includes/ $(NVHTM_CFLAGS)

# FPTREE_SRC=fptree_concurrent.c
FPTREE_OBJ=$(FPTREE_SRC:%.c=%.o)

# ALLOCATOR_SRC=allocator.c
ALLOCATOR_OBJ=$(ALLOCATOR_SRC:%.c=%.o)

THREAD_MANAGER_OBJ=$(THREAD_MANAGER_SRC:%.c=%.o)

# EXES_SRC=simple.c insert.c
# EXES=$(EXES_SRC:%.c=%.exe)

ifeq ($(NVHTM), -DNVHTM)
	NVHTM_LIB=libnh.a
endif

all: $(EXES)

%.exe:%.c $(FPTREE_OBJ) $(ALLOCATOR_OBJ) $(THREAD_MANAGER_OBJ) $(NVHTM_LIB)
	$(CXX) -o $@ $+ $(CFLAGS)

%.o:%.c
	$(CC) -c $+ $(CFLAGS)

clean:
	rm -f *.o

dist-clean: clean
	rm -f *.exe
