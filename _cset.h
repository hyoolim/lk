#ifndef CSET_H
#define CSET_H
#include "_common.h"

/* type */
typedef struct cset {
    int       capa;
    int       count;
    uint32_t  min;
    uint32_t  max;
    uint32_t *data;
    uint8_t   isinverted;
} cset_t;

/* new */
#define CSET_DEFAULTCAPA 8
cset_t *cset_alloc(void);
cset_t *cset_clone(cset_t *self);
void cset_copy(cset_t *self, cset_t *from);
void cset_fin(cset_t *self);
void cset_free(cset_t *self);
void cset_init(cset_t *self);
cset_t *cset_new(void);

/* update */
void cset_clear(cset_t *self);
void cset_add(cset_t *self, uint32_t from, uint32_t to);
void cset_subtract(cset_t *self, uint32_t from, uint32_t to);
void cset_addcset(cset_t *self, cset_t *other);
void cset_subtractcset(cset_t *self, cset_t *other);
#include "_list.h"
void cset_addstring(cset_t *self, list_t *str);
void cset_subtractstring(cset_t *self, list_t *str);

/* info */
int cset_count(const cset_t *self);
void cset_print(const cset_t *self, FILE *stream);
int cset_has(const cset_t *self, uint32_t n);
#endif
