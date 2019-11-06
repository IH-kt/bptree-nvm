USE_PMDK=-DPMDK
NO_PERSIST=-DNPERSIST

CC=gcc-9
CFLAGS=-O0 -I$(ROOT_DIR)/includes/ $(NO_PERSIST)

FPTREE_SRC=fptree.c
FPTREE_OBJ=$(FPTREE_SRC:%.c=%.o)

ALLOCATOR_SRC=allocator.c
ALLOCATOR_OBJ=$(ALLOCATOR_SRC:%.c=%.o)

# EXES_SRC=simple.c insert.c
# EXES=$(EXES_SRC:%.c=%.exe)

all: $(EXES)

%.exe:%.c $(FPTREE_OBJ) $(ALLOCATOR_OBJ)
	echo $(ROOT_DIR)
	$(CC) $(CFLAGS) -o $@ $+

%.o:%.c
	$(CC) $(CFLAGS) -c $+

clean:
	rm -f *.o

dist-clean: clean
	rm -f *.exe
