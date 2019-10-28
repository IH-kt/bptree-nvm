#ifndef H_ALLOC
#  define H_ALLOC

#include <stddef.h>
#include <stdlib.h>

#  ifndef NPERSIST
#    include <x86intrin.h>
#  endif

typedef void * ppointer;
typedef void * vpointer;
#  define P_NULL NULL
#  define V_NULL NULL

vpointer vmem_allocate(size_t);
ppointer pmem_allocate(size_t);
vpointer root_allocate(size_t, size_t);
void vmem_free(vpointer);
void pmem_free(vpointer);
void root_free(vpointer);

ppointer getPersistentAddr(vpointer);
vpointer getTransientAddr(ppointer);
#endif
