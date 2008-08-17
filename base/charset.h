#ifndef CHARSET_H
#define CHARSET_H
#include "common.h"

/* type */
typedef struct charset {
    int       capacity;
    int       size;
    uint32_t  min;
    uint32_t  max;
    uint32_t *data;
    uint8_t   isinverted;
} charset_t;

/* new */
#define CHARSET_DEFAULTCAPA 8
charset_t *charset_alloc(void);
charset_t *charset_clone(charset_t *self);
void charset_copy(charset_t *self, charset_t *from);
void charset_fin(charset_t *self);
void charset_free(charset_t *self);
void charset_init(charset_t *self);
charset_t *charset_new(void);

/* update */
void charset_clear(charset_t *self);
void charset_addRange(charset_t *self, uint32_t from, uint32_t to);
void charset_subtractRange(charset_t *self, uint32_t from, uint32_t to);
void charset_addCharSet(charset_t *self, charset_t *other);
void charset_subtractCharSet(charset_t *self, charset_t *other);
#include "darray.h"
void charset_addDArray(charset_t *self, darray_t *str);
void charset_subtractDArray(charset_t *self, darray_t *str);

/* info */
int charset_size(const charset_t *self);
void charset_print(const charset_t *self, FILE *stream);
int charset_has(const charset_t *self, uint32_t n);
#endif
