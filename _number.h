#ifndef PT_NUMBER_H
#define PT_NUMBER_H
#include "_common.h"
#include "_string.h"

/* type. */
typedef enum pt_numbertype {
    PT_NUMBERTYPE_INT = 1,
    PT_NUMBERTYPE_FLOAT,
    PT_NUMBERTYPE_NUMBER
} pt_numbertype_t;
typedef struct pt_number {
    uint8_t  *buf;
    int  len;
} pt_number_t;
typedef union pt_numberifn {
    int          i;
    double       f;
    pt_number_t *n;
} pt_numberifn_t;

/* new. */
pt_numbertype_t pt_number_new(int is_big, pt_string_t *str, pt_numberifn_t *res);
void pt_number_free(pt_number_t *self);
#endif
