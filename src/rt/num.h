#ifndef NUM_H
#define NUM_H
#include "darray.h"

typedef enum { NUM_TYPE_INT = 1, NUM_TYPE_DOUBLE } num_type_t;

typedef union {
    int i;
    double d;
} num_t;

num_type_t num_new(darray_t *str, num_t *res);
#endif
