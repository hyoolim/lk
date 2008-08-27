#ifndef CHARSET_H
#define CHARSET_H
#include "common.h"

/* type */
typedef struct {
    int       cap;
    uint32_t *data;
    uint32_t  max;
    uint32_t  min;
    int       size;
} charset_t;
#define CHARSET_DATA(self) ((uint32_t *)((ptrdiff_t)(self)->data & ~1))
#define CHARSET_ISINVERTED(self) ((ptrdiff_t)(self)->data & 1)

/* new */
charset_t *charset_alloc(void);
charset_t *charset_clone(charset_t *self);
void charset_copy(charset_t *self, charset_t *from);
void charset_fin(charset_t *self);
void charset_free(charset_t *self);
void charset_init(charset_t *self);
charset_t *charset_new(void);

/* update */
void charset_add_chars(charset_t *self, uint32_t from, uint32_t to);
void charset_add_charset(charset_t *self, charset_t *other);
void charset_clear(charset_t *self);
void charset_invert(charset_t *self);
void charset_subtract_chars(charset_t *self, uint32_t from, uint32_t to);
void charset_subtract_charset(charset_t *self, charset_t *other);

/* info */
int charset_has(const charset_t *self, uint32_t achar);
void charset_inspect(const charset_t *self, FILE *output);
int charset_size(const charset_t *self);

/* delayed due to cross-dependency */
#include "darray.h"
void charset_add_darray(charset_t *self, darray_t *str);
void charset_subtract_darray(charset_t *self, darray_t *str);
darray_t *charset_tostr(const charset_t *self);
#endif
