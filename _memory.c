#include "_memory.h"

/* new */
static int alloccount = 0;
static size_t alloctotal = 0;
static size_t allocused = 0;
static size_t allocpeak = 0;
static void *recycled[MEMORY_MAXRECYCLED];
void *memory_alloc(size_t size) {
    alloccount ++;
    if(size < MEMORY_MAXRECYCLED && recycled[size] != NULL) {
        void *next = *(void **)recycled[size];
        void *new = recycled[size];
        recycled[size] = next;
        memset(new, 0x0, size);
        return new;
    } else {
        size_t *new = calloc(1, size + sizeof(size_t));
        if(new == NULL) MEMORYERROR("Unable to allocate");
        *new = size;
        alloctotal += size;
        allocused += size;
        if(allocused > allocpeak) allocpeak = allocused;
        return new + 1;
    }
}
void memory_free(void *ptr) {
    if(ptr != NULL) {
        int size = *((size_t *)ptr - 1);
        alloccount --;
        if(size < MEMORY_MAXRECYCLED) {
            *(void **)ptr = recycled[size];
            recycled[size] = ptr;
        } else {
            int size2 = *(size_t *)(ptr = (size_t *)ptr - 1);
            free(ptr);
            allocused -= size2;
        }
    }
}
void memory_freerecycled(void) {
    int i;
    void *curr;
    for(i = 0; i < MEMORY_MAXRECYCLED; i ++) {
        curr = recycled[i];
        if(curr != NULL) {
            while(curr != NULL) {
                recycled[i] = *(void **)curr;
                free((size_t *)curr - 1);
                allocused -= i;
                curr = recycled[i];
            }
        }
    }
}
void *memory_resize(void *old, size_t size) {
    if(old == NULL) return memory_alloc(size);
    else {
        int old_size = *(size_t *)(old = (size_t *)old - 1);
        size_t *new = realloc(old, size + sizeof(size_t));
        if(new == NULL) MEMORYERROR("Unable to resize");
        *new = size;
        alloctotal += size - old_size;
        allocused += size - old_size;
        if(allocused > allocpeak) allocpeak = allocused;
        return new + 1;
    }
}

/* info */
int memory_alloccount(void) {
    return alloccount;
}
size_t memory_alloctotal(void) {
    return alloctotal;
}
size_t memory_allocused(void) {
    return allocused;
}
size_t memory_allocpeak(void) {
    return allocpeak;
}
