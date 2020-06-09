#ifdef USE_PMEM
#include <pthread.h>
void *stamp_mapped_file_pointer = 0;
stamp_used_bytes = 0;
pthread_mutex_t tm_malloc_mutex;
#endif
