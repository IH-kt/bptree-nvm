#ifndef H_ALLOC
#  define H_ALLOC

#include <stddef.h>
#include <stdlib.h>

#  ifndef NPERSIST
#    include <x86intrin.h>
#  endif

typedef void * ppointer;
#  define P_NULL NULL
#  define V_NULL NULL

void *vmem_allocate(size_t);
ppointer pmem_allocate(size_t);
void *root_allocate(size_t, size_t);
void vmem_free(void *);
void pmem_free(ppointer);
void root_free(ppointer);

ppointer getPersistentAddr(void *);
void *getTransientAddr(ppointer);
#endif
