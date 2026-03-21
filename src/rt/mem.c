#include "mem.h"
#include <stdatomic.h>

// Stats use relaxed ordering — diagnostics, not synchronization points
static _Atomic int alloc_count = 0;
static _Atomic size_t alloc_total = 0;
static _Atomic size_t alloc_used = 0;
static _Atomic size_t alloc_peak = 0;
static _Atomic size_t alloc_recycled = 0;

// Per-thread free lists indexed by allocation size. Thread-local means the
// hot path (small allocs/frees) is fully lock-free with no cross-thread contention.
// Blocks freed on one thread are not reused by another
static _Thread_local void *recycled[MEM_MAX_RECYCLED];

// CAS loop because the check and update must be atomic together
static void update_peak(void) {
    size_t used = atomic_load_explicit(&alloc_used, memory_order_relaxed);
    size_t peak = atomic_load_explicit(&alloc_peak, memory_order_relaxed);

    while (used > peak) {
        if (atomic_compare_exchange_weak_explicit(&alloc_peak, &peak, used, memory_order_relaxed, memory_order_relaxed))
            break;
    }
}

void *mem_alloc(size_t size) {
    assert(size > 0);
    atomic_fetch_add_explicit(&alloc_count, 1, memory_order_relaxed);

    if (size < MEM_MAX_RECYCLED && recycled[size] != NULL) {
        // Pop from free list — first word of each recycled block stores the next pointer
        void *next = *(void **)recycled[size];
        void *block = recycled[size];
        recycled[size] = next;
        memset(block, 0x0, size); // Re-zero to match calloc semantics
        atomic_fetch_add_explicit(&alloc_used, size, memory_order_relaxed);
        atomic_fetch_sub_explicit(&alloc_recycled, size, memory_order_relaxed);
        update_peak();
        return block;
    }

    size_t *header = calloc(1, size + sizeof(size_t));

    if (header == NULL)
        ERR("Unable to allocate mem!");

    *header = size; // Size header
    atomic_fetch_add_explicit(&alloc_total, size, memory_order_relaxed);
    atomic_fetch_add_explicit(&alloc_used, size, memory_order_relaxed);
    update_peak();
    return header + 1;
}

void mem_free(void *block) {
    if (block != NULL) {
        size_t size = *((size_t *)block - 1);
        atomic_fetch_sub_explicit(&alloc_count, 1, memory_order_relaxed);

        if (size < MEM_MAX_RECYCLED) {
            // Push onto free list — threads the next pointer through the block
            *(void **)block = recycled[size];
            recycled[size] = block;
            atomic_fetch_sub_explicit(&alloc_used, size, memory_order_relaxed);
            atomic_fetch_add_explicit(&alloc_recycled, size, memory_order_relaxed);

        } else {
            free((size_t *)block - 1);
            atomic_fetch_sub_explicit(&alloc_used, size, memory_order_relaxed);
        }
    }
}

// Releases the calling thread's recycler — each thread owns its own,
// so callers on different threads free independent sets of blocks
void mem_free_recycled(void) {
    for (int i = 0; i < MEM_MAX_RECYCLED; i++) {
        void *current = recycled[i];

        while (current != NULL) {
            recycled[i] = *(void **)current;
            free((size_t *)current - 1);
            atomic_fetch_sub_explicit(&alloc_recycled, i, memory_order_relaxed);
            current = recycled[i];
        }
    }
}

void *mem_resize(void *old, size_t size) {
    if (old == NULL)
        return mem_alloc(size);

    assert(size > 0);
    size_t *old_header = (size_t *)old - 1;
    size_t old_size = *old_header;

    if (size == old_size)
        return old;

    if (size < MEM_MAX_RECYCLED && recycled[size] != NULL) {
        // Serve new size from recycler, copy data, then recycle or free old
        void *next = *(void **)recycled[size];
        void *block = recycled[size];
        recycled[size] = next;
        size_t copy_size = old_size < size ? old_size : size;
        memcpy(block, old, copy_size);

        if (size > old_size)
            memset((char *)block + old_size, 0, size - old_size); // Zero-init growth

        atomic_fetch_sub_explicit(&alloc_recycled, size, memory_order_relaxed);

        if (old_size < MEM_MAX_RECYCLED) {
            *(void **)old = recycled[old_size];
            recycled[old_size] = old;
            atomic_fetch_add_explicit(&alloc_recycled, old_size, memory_order_relaxed);

        } else {
            free(old_header);
        }

        if (size > old_size) {
            atomic_fetch_add_explicit(&alloc_total, size - old_size, memory_order_relaxed);
            atomic_fetch_add_explicit(&alloc_used, size - old_size, memory_order_relaxed);

        } else {
            atomic_fetch_sub_explicit(&alloc_used, old_size - size, memory_order_relaxed);
        }

        update_peak();
        return block;
    }

    size_t *header = realloc(old_header, size + sizeof(size_t));

    if (header == NULL)
        ERR("Unable to resize mem!");

    *header = size;

    if (size > old_size) {
        atomic_fetch_add_explicit(&alloc_total, size - old_size, memory_order_relaxed);
        atomic_fetch_add_explicit(&alloc_used, size - old_size, memory_order_relaxed);

    } else {
        atomic_fetch_sub_explicit(&alloc_used, old_size - size, memory_order_relaxed);
    }

    update_peak();
    return header + 1;
}

// info
int mem_alloc_count(void) {
    return atomic_load_explicit(&alloc_count, memory_order_relaxed);
}

size_t mem_alloc_total(void) {
    return atomic_load_explicit(&alloc_total, memory_order_relaxed);
}

size_t mem_alloc_used(void) {
    return atomic_load_explicit(&alloc_used, memory_order_relaxed);
}

size_t mem_alloc_peak(void) {
    return atomic_load_explicit(&alloc_peak, memory_order_relaxed);
}

size_t mem_alloc_recycled(void) {
    return atomic_load_explicit(&alloc_recycled, memory_order_relaxed);
}
