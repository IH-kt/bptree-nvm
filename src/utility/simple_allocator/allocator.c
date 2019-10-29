#include "allocator.h"
void *vmem_allocate(size_t size) {
    return malloc(size);
}
ppointer pmem_allocate(size_t size) {
    return malloc(size);
}
void *root_allocate(size_t size, size_t element_size) {
    return pmem_allocate(size);
}

void vmem_free(void *p) {
    free(p);
}
void pmem_free(ppointer p) {
    free(p);
}
void root_free(ppointer p) {
    free(p);
}

ppointer getPersistentAddr(void *p) {return p;}
void *getTransientAddr(ppointer p) {return p;}
