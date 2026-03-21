#include "mem.h"

// new
static int allocsize = 0;
static size_t alloctotal = 0;
static size_t allocused = 0;
static size_t allocpeak = 0;
static size_t allocrecycled = 0;
static void *recycled[MEMORY_MAXRECYCLED];

void *mem_alloc(size_t size) {
    assert(size > 0);
    allocsize++;

    if (size < MEMORY_MAXRECYCLED && recycled[size] != NULL) {
        void *next = *(void **)recycled[size];
        void *new = recycled[size];
        recycled[size] = next;
        memset(new, 0x0, size);
        allocused += size;
        allocrecycled -= size;

        if (allocused > allocpeak)
            allocpeak = allocused;
        return new;

    } else {
        size_t *new = calloc(1, size + sizeof(size_t));

        if (new == NULL)
            ERR("Unable to allocate mem!");
        *new = size;
        alloctotal += size;
        allocused += size;

        if (allocused > allocpeak)
            allocpeak = allocused;
        return new + 1;
    }
}

void mem_free(void *ptr) {
    if (ptr != NULL) {
        size_t size = *((size_t *)ptr - 1);
        allocsize--;

        if (size < MEMORY_MAXRECYCLED) {
            *(void **)ptr = recycled[size];
            recycled[size] = ptr;
            allocused -= size;
            allocrecycled += size;

        } else {
            size_t size2 = *(size_t *)(ptr = (size_t *)ptr - 1);
            free(ptr);
            allocused -= size2;
        }
    }
}

void mem_freerecycled(void) {
    int i;
    void *curr;

    for (i = 0; i < MEMORY_MAXRECYCLED; i++) {
        curr = recycled[i];

        if (curr != NULL) {
            while (curr != NULL) {
                recycled[i] = *(void **)curr;
                free((size_t *)curr - 1);
                allocrecycled -= i;
                curr = recycled[i];
            }
        }
    }
}

void *mem_resize(void *old, size_t size) {
    if (old == NULL)
        return mem_alloc(size);

    assert(size > 0);

    size_t *old_header = (size_t *)old - 1;
    size_t old_size = *old_header;

    if (size < MEMORY_MAXRECYCLED && recycled[size] != NULL) {
        void *next = *(void **)recycled[size];
        void *new = recycled[size];
        recycled[size] = next;
        size_t copy_size = old_size < size ? old_size : size;
        memcpy(new, old, copy_size);

        if (size > old_size)
            memset((char *)new + old_size, 0, size - old_size);
        allocrecycled -= size;

        if (old_size < MEMORY_MAXRECYCLED) {
            *(void **)old = recycled[old_size];
            recycled[old_size] = old;
            allocrecycled += old_size;
        } else {
            free(old_header);
        }

        if (size > old_size) {
            alloctotal += size - old_size;
            allocused += size - old_size;
        } else {
            allocused -= old_size - size;
        }

        if (allocused > allocpeak)
            allocpeak = allocused;
        return new;
    }

    size_t *new = realloc(old_header, size + sizeof(size_t));

    if (new == NULL)
        ERR("Unable to resize mem!");
    *new = size;

    if (size > old_size) {
        alloctotal += size - old_size;
        allocused += size - old_size;
    } else {
        allocused -= old_size - size;
    }

    if (allocused > allocpeak)
        allocpeak = allocused;
    return new + 1;
}

// info
int mem_allocsize(void) {
    return allocsize;
}

size_t mem_alloctotal(void) {
    return alloctotal;
}

size_t mem_allocused(void) {
    return allocused;
}

size_t mem_allocpeak(void) {
    return allocpeak;
}

size_t mem_allocrecycled(void) {
    return allocrecycled;
}
