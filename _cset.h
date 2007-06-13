#ifndef PT_CSET_H
#define PT_CSET_H
#include "_common.h"

/* type */
typedef struct pt_cset {
    int       capa;
    int       count;
    uint32_t  min;
    uint32_t  max;
    uint32_t *data;
    uint8_t   isinverted;
} pt_cset_t;
#define PT_CSET(o) ((pt_cset_t *)(o))

/* new */
#define PT_CSET_DEFAULTCAPA 8
pt_cset_t *pt_cset_alloc(void);
pt_cset_t *pt_cset_clone(pt_cset_t *self);
void pt_cset_copy(pt_cset_t *self, pt_cset_t *from);
void pt_cset_fin(pt_cset_t *self);
void pt_cset_free(pt_cset_t *self);
void pt_cset_init(pt_cset_t *self);
pt_cset_t *pt_cset_new(void);

/* update */
void pt_cset_clear(pt_cset_t *self);
void pt_cset_add(pt_cset_t *self, uint32_t from, uint32_t to);
void pt_cset_subtract(pt_cset_t *self, uint32_t from, uint32_t to);
void pt_cset_addcset(pt_cset_t *self, pt_cset_t *other);
void pt_cset_subtractcset(pt_cset_t *self, pt_cset_t *other);
#include "_list.h"
void pt_cset_addstring(pt_cset_t *self, pt_list_t *str);
void pt_cset_subtractstring(pt_cset_t *self, pt_list_t *str);

/* info */
int pt_cset_count(const pt_cset_t *self);
void pt_cset_print(const pt_cset_t *self, FILE *stream);
int pt_cset_has(const pt_cset_t *self, uint32_t n);
#endif
