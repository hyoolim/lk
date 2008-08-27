#ifndef NUMBER_H
#define NUMBER_H
#include "common.h"
#include "darray.h"

/* type. */
typedef enum numtype {
    NUMBERTYPE_INT = 1,
    NUMBERTYPE_FLOAT,
    NUMBERTYPE_NUMBER
} numtype_t;
typedef struct num {
    uint8_t  *buf;
    int  len;
} num_t;
typedef union numifn {
    int          i;
    double       f;
    num_t *n;
} numifn_t;

/* new. */
numtype_t num_new(int is_big, darray_t *str, numifn_t *res);
void num_free(num_t *self);
#endif
