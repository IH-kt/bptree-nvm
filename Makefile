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
CFLAGS=-O0 -g -march=native -pthread -I$(ROOT_DIR)/includes/ $(CLWB) $(CONCURRENT) $(NO_PERSIST) $(ALLOCATOR) $(TIME_PART)

# FPTREE_SRC=fptree_concurrent.c
FPTREE_OBJ=$(FPTREE_SRC:%.c=%.o)

# ALLOCATOR_SRC=allocator.c
ALLOCATOR_OBJ=$(ALLOCATOR_SRC:%.c=%.o)

THREAD_MANAGER_OBJ=$(THREAD_MANAGER_SRC:%.c=%.o)

# EXES_SRC=simple.c insert.c
# EXES=$(EXES_SRC:%.c=%.exe)

all: $(EXES)

%.exe:%.c $(FPTREE_OBJ) $(ALLOCATOR_OBJ) $(THREAD_MANAGER_OBJ)
	$(CC) $(CFLAGS) -o $@ $+

%.o:%.c
	$(CC) $(CFLAGS) -c $+

clean:
	rm -f *.o

dist-clean: clean
	rm -f *.exe
