#ifndef PT_MEMORY_H
#define PT_MEMORY_H
#include "_common.h"
#include <string.h>

/* new */
#define PT_MEMORY_MAXRECYCLED 1000
void *pt_memory_alloc(size_t size);
void pt_memory_free(void *ptr);
void pt_memory_freerecycled(void);
void *pt_memory_resize(void *old, size_t size);

/* info */
size_t pt_memory_alloctotal(void);
size_t pt_memory_allocused(void);
size_t pt_memory_allocpeak(void);
#endif
