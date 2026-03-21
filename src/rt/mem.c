#include "mem.h"
#include <stdatomic.h>

// new
static _Atomic int allocsize = 0;
static _Atomic size_t alloctotal = 0;
static _Atomic size_t allocused = 0;
static _Atomic size_t allocpeak = 0;
static _Atomic size_t allocrecycled = 0;
static _Thread_local void *recycled[MEMORY_MAXRECYCLED];

static void update_peak(void) {
    size_t used = atomic_load_explicit(&allocused, memory_order_relaxed);
    size_t peak = atomic_load_explicit(&allocpeak, memory_order_relaxed);

    while (used > peak) {
        if (atomic_compare_exchange_weak_explicit(&allocpeak, &peak, used,
                memory_order_relaxed, memory_order_relaxed))
            break;
    }
}

void *mem_alloc(size_t size) {
    assert(size > 0);
    atomic_fetch_add_explicit(&allocsize, 1, memory_order_relaxed);

    if (size < MEMORY_MAXRECYCLED && recycled[size] != NULL) {
        void *next = *(void **)recycled[size];
        void *new = recycled[size];
        recycled[size] = next;
        memset(new, 0x0, size);
        atomic_fetch_add_explicit(&allocused, size, memory_order_relaxed);
        atomic_fetch_sub_explicit(&allocrecycled, size, memory_order_relaxed);
        update_peak();
        return new;

    } else {
        size_t *new = calloc(1, size + sizeof(size_t));

        if (new == NULL)
            ERR("Unable to allocate mem!");
        *new = size;
        atomic_fetch_add_explicit(&alloctotal, size, memory_order_relaxed);
        atomic_fetch_add_explicit(&allocused, size, memory_order_relaxed);
        update_peak();
        return new + 1;
    }
}

void mem_free(void *ptr) {
    if (ptr != NULL) {
        size_t size = *((size_t *)ptr - 1);
        atomic_fetch_sub_explicit(&allocsize, 1, memory_order_relaxed);

        if (size < MEMORY_MAXRECYCLED) {
            *(void **)ptr = recycled[size];
            recycled[size] = ptr;
            atomic_fetch_sub_explicit(&allocused, size, memory_order_relaxed);
            atomic_fetch_add_explicit(&allocrecycled, size, memory_order_relaxed);

        } else {
            size_t size2 = *(size_t *)(ptr = (size_t *)ptr - 1);
            free(ptr);
            atomic_fetch_sub_explicit(&allocused, size2, memory_order_relaxed);
        }
    }
}

void mem_freerecycled(void) {
    for (int i = 0; i < MEMORY_MAXRECYCLED; i++) {
        void *curr = recycled[i];

        if (curr != NULL) {
            while (curr != NULL) {
                recycled[i] = *(void **)curr;
                free((size_t *)curr - 1);
                atomic_fetch_sub_explicit(&allocrecycled, i, memory_order_relaxed);
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
        atomic_fetch_sub_explicit(&allocrecycled, size, memory_order_relaxed);

        if (old_size < MEMORY_MAXRECYCLED) {
            *(void **)old = recycled[old_size];
            recycled[old_size] = old;
            atomic_fetch_add_explicit(&allocrecycled, old_size, memory_order_relaxed);
        } else {
            free(old_header);
        }

        if (size > old_size) {
            atomic_fetch_add_explicit(&alloctotal, size - old_size, memory_order_relaxed);
            atomic_fetch_add_explicit(&allocused, size - old_size, memory_order_relaxed);
        } else {
            atomic_fetch_sub_explicit(&allocused, old_size - size, memory_order_relaxed);
        }

        update_peak();
        return new;
    }

    size_t *new = realloc(old_header, size + sizeof(size_t));

    if (new == NULL)
        ERR("Unable to resize mem!");
    *new = size;

    if (size > old_size) {
        atomic_fetch_add_explicit(&alloctotal, size - old_size, memory_order_relaxed);
        atomic_fetch_add_explicit(&allocused, size - old_size, memory_order_relaxed);
    } else {
        atomic_fetch_sub_explicit(&allocused, old_size - size, memory_order_relaxed);
    }

    update_peak();
    return new + 1;
}

// info
int mem_allocsize(void) {
    return atomic_load_explicit(&allocsize, memory_order_relaxed);
}

size_t mem_alloctotal(void) {
    return atomic_load_explicit(&alloctotal, memory_order_relaxed);
}

size_t mem_allocused(void) {
    return atomic_load_explicit(&allocused, memory_order_relaxed);
}

size_t mem_allocpeak(void) {
    return atomic_load_explicit(&allocpeak, memory_order_relaxed);
}

size_t mem_allocrecycled(void) {
    return atomic_load_explicit(&allocrecycled, memory_order_relaxed);
}
