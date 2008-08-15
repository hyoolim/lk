#ifndef MEMORY_H
#define MEMORY_H
#include "common.h"
#include <string.h>

/* new */
#define MEMORY_MAXRECYCLED 1000
void *memory_alloc(size_t size);
void memory_free(void *ptr);
void memory_freerecycled(void);
void *memory_resize(void *old, size_t size);

/* info */
int memory_alloccount(void);
size_t memory_alloctotal(void);
size_t memory_allocused(void);
size_t memory_allocpeak(void);
#endif
