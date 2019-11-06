USE_PMDK=-DPMDK
NO_PERSIST=-DNPERSIST
CLWB=-DCLWB
CONCURRENT=-DCONCURRENT

CC=gcc
CFLAGS=-O0 -g -march=native -I$(ROOT_DIR)/includes/ $(CLWB) $(CONCURRENT)

FPTREE_SRC=fptree_concurrent.c
FPTREE_OBJ=$(FPTREE_SRC:%.c=%.o)

ALLOCATOR_SRC=allocator.c
ALLOCATOR_OBJ=$(ALLOCATOR_SRC:%.c=%.o)

# EXES_SRC=simple.c insert.c
# EXES=$(EXES_SRC:%.c=%.exe)

all: $(EXES)

%.exe:%.c $(FPTREE_OBJ) $(ALLOCATOR_OBJ)
	$(CC) $(CFLAGS) -o $@ $+

%.o:%.c
	$(CC) $(CFLAGS) -c $+

clean:
	rm -f *.o

dist-clean: clean
	rm -f *.exe
