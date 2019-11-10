#include "allocator.h"

PAddr PADDR_NULL = { 0, 0 };

#ifdef ALLOCTIME
struct timespec myalloc_start_time = {0, 0};
struct timespec myalloc_finish_time = {0, 0};
long long allocate_time = 0;
long long persist_time = 0;
#endif

#define DEFAULT_NODE_NUM 3 // デフォルトでフリーリストに入っている数

typedef struct AllocatorHeader {
    PAddr node_head;
} AllocatorHeader;

typedef struct FreeNode {
    struct FreeNode *next;
    void *node;
} FreeNode;

typedef struct MemoryRoot {
    unsigned char global_lock;
    void *global_free_area_head;
    size_t remaining_amount;
    FreeNode *global_free_list_head; // for after recovery
    unsigned char **list_lock;
    FreeNode ***local_free_list_head_ary; // [i][j] -> スレッドiの持つスレッドj用のフリーリストへのポインタ
    FreeNode ***local_free_list_tail_ary; // [i][j] -> スレッドiの持つスレッドj用のフリーリスト末尾へのポインタ
} MemoryRoot;

/*
 * 構造：
 * | head_ppointer | root_ppointer | node[0] | ...
 * 新規作成の場合head_ppointer = {0, 0}
 * fidは1から．現在のところ複数ファイルには対応していない．
 * また最初に確保した以上のメモリを確保することもない．
 * nスレッドごとにn * nのフリーリストを持ち，ノードのスレッドidと同じリストに返す
 */

void *_pmem_mmap_head = NULL;
void *_pmem_user_head = NULL;
size_t _pmem_mmap_size = 0;
size_t _pmem_user_size = 0;
int _number_of_thread = -1;
MemoryRoot *_pmem_memory_root = NULL;
size_t _tree_node_size = 0;

void *getFromArea(void **head, size_t node_size) {
    // while(!__sync_bool_compare_and_swap(&lock, 0, 1));
    void *new = *head;
    *head += node_size;
    return new;
}

FreeNode *getFromList(FreeNode **head) {
    if (*head != NULL) {
        FreeNode *node;
        node = *head;
        *head = node->next;
        return node;
    }
    return NULL;
}

void addToList(FreeNode *node, FreeNode **head, FreeNode **tail) {
    if (*tail != NULL) {
        (*tail)->next = node;
    } else {
        *head = node;
    }
    node->next = NULL;
    *tail = node;
}

size_t addDefaultEmptyNode(FreeNode **head, FreeNode **tail, FreeNode *global_list_head, size_t pmem_remain, size_t node_size, void **free_head) {
    int i;
    size_t used_amount = 0;
    for (i = 0; i < DEFAULT_NODE_NUM; i++) {
        if (global_list_head != NULL) {
            // if recovery found free node
        } else {
            if (pmem_remain >= node_size) {
                FreeNode *new = (FreeNode *)vmem_allocate(sizeof(FreeNode));
                new->next = NULL;
                new->node = getFromArea(free_head, node_size);
                addToList(new, head, tail);
                pmem_remain -= node_size;
                used_amount += node_size;
            } else {
                return -1; // out of memory
            }
        }
    }
    return used_amount;
}

void initMemoryRoot(MemoryRoot *mr, unsigned char thread_num, void *head, size_t pmem_size, size_t node_size, FreeNode *global_list_head) {
    mr->global_free_area_head = head;
    mr->global_free_list_head = global_list_head;
    mr->remaining_amount = pmem_size;
    mr->local_free_list_head_ary = (FreeNode ***)vmem_allocate(sizeof(FreeNode **) * thread_num);
    mr->local_free_list_tail_ary = (FreeNode ***)vmem_allocate(sizeof(FreeNode **) * thread_num);
    mr->list_lock = (unsigned char **)vmem_allocate(sizeof(unsigned char *) * thread_num);
    int i, j;
    for (i = 0; i < thread_num; i++) {
        mr->local_free_list_head_ary[i] = (FreeNode **)vmem_allocate(sizeof(FreeNode *) * thread_num);
        mr->local_free_list_tail_ary[i] = (FreeNode **)vmem_allocate(sizeof(FreeNode *) * thread_num);
        mr->list_lock[i] = (unsigned char *)vmem_allocate(sizeof(unsigned char) * thread_num);
        for (j = 0; j < thread_num; j++) {
            mr->local_free_list_head_ary[i][j] = NULL;
            mr->local_free_list_tail_ary[i][j] = NULL;
            if (i == j) {
                addDefaultEmptyNode(&mr->local_free_list_head_ary[i][j], &mr->local_free_list_tail_ary[i][j],
                    mr->global_free_list_head, mr->remaining_amount, node_size, &mr->global_free_area_head);
            }
            mr->list_lock[i][j] = 0;
        }
    }
}
void destroyMemoryRoot(MemoryRoot *mr) {
    // TODO: need to traverse free list
    int i;
    for (i = 0; i < _number_of_thread; i++) {
        vmem_free(mr->local_free_list_head_ary[i]);
        vmem_free(mr->local_free_list_tail_ary[i]);
        vmem_free(mr->list_lock[i]);
    }
    vmem_free(mr->local_free_list_head_ary);
    vmem_free(mr->local_free_list_tail_ary);
    vmem_free(mr->list_lock);
}

int comparePAddr(PAddr a, PAddr b) {
    return (a.fid == b.fid) && (a.offset == b.offset);
}

ppointer getPersistentAddr(void *addr) {
    ppointer paddr;
    if (addr == NULL) {
        return PADDR_NULL;
    } else {
        paddr.fid = 1;
        paddr.offset = addr - _pmem_mmap_head;
        return paddr;
    }
}

void *getTransientAddr(ppointer paddr) {
    if (comparePAddr(paddr, PADDR_NULL)) {
        return NULL;
    } else {
        return _pmem_mmap_head + paddr.offset;
    }
}

int initAllocator(const char *path, size_t pmem_size, unsigned char thread_num) {
    struct stat fi;
    int err;
    int fd = open(path, O_RDWR);
    if (fd == -1) {
        if (errno == ENOENT) {
            AllocatorHeader new_header;
            new_header.node_head = PADDR_NULL;
            fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
            int written_size = write(fd, &new_header, sizeof(AllocatorHeader));
            if (written_size < sizeof(ppointer)) {
                perror("write");
                return -1;
            }
            lseek(fd, 0, SEEK_SET);
        }
        if (fd == -1) {
            perror("open");
            return -1;
        }
    }

    // extend file
    if (fstat(fd, &fi) == -1) {
        perror("fstat");
        return -1;
    }
    if (fi.st_size < pmem_size) {
        err = ftruncate(fd, pmem_size);
        if (err != 0) {
            perror("ftruncate");
            return -1;
        }
    }

    _pmem_mmap_head = mmap(NULL, pmem_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (_pmem_mmap_head == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    _pmem_mmap_size = pmem_size;
    _pmem_user_head = _pmem_mmap_head + sizeof(AllocatorHeader);
    _pmem_user_size = pmem_size - sizeof(AllocatorHeader);

    // fd can be closed after mmap
    err = close(fd);
    if (err == -1) {
        perror("close");
        return -1;
    }

    _number_of_thread = thread_num;

    return 0;
}

ppointer getHeadPPointer() {
    if (_pmem_user_head == NULL) {
        return PADDR_NULL;
    }

    ppointer *head_pointer = (ppointer *)_pmem_user_head;
    return *head_pointer;
}

ppointer recoverAllocator(ppointer (*getNext)(ppointer)) {
    // TODO: implement recovery
    // find free node and make global free list
    return getHeadPPointer();
};

ppointer *root_allocate(size_t size, size_t node_size) {
#ifdef ALLOCTIME
    clock_gettime(CLOCK_MONOTONIC, &myalloc_start_time);
#endif
    _pmem_memory_root = (MemoryRoot *)vmem_allocate(sizeof(MemoryRoot));
    initMemoryRoot(_pmem_memory_root, _number_of_thread, _pmem_user_head + size, _pmem_user_size - size, node_size, NULL);
#ifdef ALLOCTIME
    clock_gettime(CLOCK_MONOTONIC, &myalloc_finish_time);
    myalloc_allocate_time += (myalloc_finish_time.tv_sec - myalloc_start_time.tv_sec) * 1000000000L + (myalloc_finish_time.tv_nsec - myalloc_start_time.tv_nsec);
#endif
    _tree_node_size = node_size;
    ppointer *root_p = (ppointer *)vmem_allocate(sizeof(ppointer));
    *root_p = getPersistentAddr(_pmem_user_head);
    return root_p;
}

void root_free(ppointer *root) {
    vmem_free(root);
}

ppointer pmem_allocate(size_t size, unsigned char tid) {
    tid--;
    FreeNode *free_node;
    void *new_node;
    // assert (size == _tree_node_size);
    if (_pmem_memory_root->local_free_list_tail_ary[tid][tid] != NULL &&
        _pmem_memory_root->local_free_list_tail_ary[tid][tid] != _pmem_memory_root->local_free_list_head_ary[tid][tid]) {
        free_node = getFromList(&_pmem_memory_root->local_free_list_head_ary[tid][tid]);
        new_node = free_node->node;
        vmem_free(free_node);
    } else {
        int i;
        for (i = 0; i < _number_of_thread; i++) {
            if (_pmem_memory_root->local_free_list_tail_ary[i][tid] != NULL &&
                _pmem_memory_root->local_free_list_tail_ary[i][tid] != _pmem_memory_root->local_free_list_head_ary[i][tid]) {
                while (!__sync_bool_compare_and_swap(&_pmem_memory_root->list_lock[i][tid], 0, 1));
                free_node = getFromList(&_pmem_memory_root->local_free_list_head_ary[i][tid]);
                _pmem_memory_root->list_lock[i][tid] = 0;
                new_node = free_node->node;
                vmem_free(free_node);
                break;
            }
        }
        if (i == _number_of_thread) {
            while (!__sync_bool_compare_and_swap(&_pmem_memory_root->global_lock, 0, 1));
            new_node = getFromArea(&_pmem_memory_root->global_free_area_head, _tree_node_size);
            _pmem_memory_root->global_lock = 0;
        }
    }
    return getPersistentAddr(new_node);
}

void pmem_free(ppointer node, unsigned char node_tid, unsigned char tid) {
    node_tid--;
    tid--;
    FreeNode *new_free = (FreeNode *)vmem_allocate(sizeof(FreeNode));
    new_free->node = getTransientAddr(node);
    while (!__sync_bool_compare_and_swap(&_pmem_memory_root->list_lock[tid][node_tid], 0, 1));
    addToList(new_free, &_pmem_memory_root->local_free_list_head_ary[tid][node_tid], &_pmem_memory_root->local_free_list_tail_ary[tid][node_tid]);
    _pmem_memory_root->list_lock[tid][node_tid] = 0;
}

void *vmem_allocate(size_t size) {
    return malloc(size);
}

void vmem_free(void *p) {
    free(p);
}

int destroyAllocator() {
    destroyMemoryRoot(_pmem_memory_root);
    vmem_free(_pmem_memory_root);

#ifdef ALLOCTIME
    printf("alloctime:%lld.%09lld\n", myalloc_allocate_time/1000000000L, myalloc_allocate_time%1000000000L);
    printf("persisttime:%lld.%09lld\n", myalloc_persist_time/1000000000L, myalloc_persist_time%1000000000L);
#endif

    int err = munmap(_pmem_mmap_head, _pmem_mmap_size);
    if (err == -1) {
        perror("munmap");
        return -1;
    }
    return 0;
}
