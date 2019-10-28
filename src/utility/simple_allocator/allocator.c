#include "allocator.h"
vpointer vmem_allocate(size_t size) {
    return malloc(size);
}
ppointer pmem_allocate(size_t size) {
    return malloc(size);
}
vpointer root_allocate(size_t size, size_t element_size) {
    return pmem_allocate(size);
}

void vmem_free(vpointer p) {
    free(p);
}
void pmem_free(vpointer p) {
    free(p);
}
void root_free(vpointer p) {
    free(p);
}

ppointer getPersistentAddr(vpointer p) {return p;}
vpointer getTransientAddr(ppointer p) {return p;}
