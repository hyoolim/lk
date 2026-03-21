#ifndef MEM_H
#define MEM_H
#include "common.h"
#include <string.h>

// Allocator — each allocation is prefixed with a size_t header:
//   [ size_t size | user data ... ]
//                   ^-- returned pointer
// Small allocations (size < MEM_MAX_RECYCLED) are kept in a per-thread
// free list indexed by size, avoiding OS calls on reuse
#define MEM_MAX_RECYCLED 1000
void *mem_alloc(size_t size);
void mem_free(void *ptr);
void mem_free_recycled(void); // Release the calling thread's recycler to the OS
void *mem_resize(void *old, size_t size);

// Diagnostic stats (relaxed atomics — approximate under concurrency)
int mem_alloc_count(void);       // Live allocation count
size_t mem_alloc_total(void);    // Cumulative bytes ever allocated (grows only)
size_t mem_alloc_used(void);     // Bytes in live allocations
size_t mem_alloc_peak(void);     // Peak value of alloc_used
size_t mem_alloc_recycled(void); // Bytes held in recycler (not live)
#endif
