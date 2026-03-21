#ifndef MEMORY_H
#define MEMORY_H
#include "common.h"
#include <string.h>

/* new */
#define MEMORY_MAXRECYCLED 1000
void *mem_alloc(size_t size);
void mem_free(void *ptr);
void mem_freerecycled(void);
void *mem_resize(void *old, size_t size);

/* info */
int mem_allocsize(void);
size_t mem_alloctotal(void);
size_t mem_allocused(void);
size_t mem_allocpeak(void);
#endif
