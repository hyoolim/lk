#ifndef NUM_H
#define NUM_H
#include "common.h"
#include "darray.h"

typedef enum { NUM_TYPE_INT = 1, NUM_TYPE_DOUBLE, NUM_TYPE_BIG } num_type_t;

typedef struct {
    uint8_t *buf;
    size_t len;
} num_big_t;

typedef union {
    int i;
    double d;
    num_big_t *b;
} num_t;

num_type_t num_new(bool big, darray_t *str, num_t *res);
void num_free(num_big_t *self);
#endif
