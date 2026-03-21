#ifndef MEM_H
#define MEM_H
#include "common.h"
#include <string.h>

// new
#define MEMORY_MAX_RECYCLED 1000
void *mem_alloc(size_t size);
void mem_free(void *ptr);
void mem_free_recycled(void);
void *mem_resize(void *old, size_t size);

// info
int mem_alloc_count(void);
size_t mem_alloc_total(void);
size_t mem_alloc_used(void);
size_t mem_alloc_peak(void);
size_t mem_alloc_recycled(void);
#endif
