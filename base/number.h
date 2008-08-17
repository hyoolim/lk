#ifndef NUMBER_H
#define NUMBER_H
#include "common.h"
#include "string.h"

/* type. */
typedef enum numbertype {
    NUMBERTYPE_INT = 1,
    NUMBERTYPE_FLOAT,
    NUMBERTYPE_NUMBER
} numbertype_t;
typedef struct number {
    uint8_t  *buf;
    int  len;
} number_t;
typedef union numberifn {
    int          i;
    double       f;
    number_t *n;
} numberifn_t;

/* new. */
numbertype_t number_new(int is_big, array_t *str, numberifn_t *res);
void number_free(number_t *self);
#endif
