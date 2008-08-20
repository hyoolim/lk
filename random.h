#ifndef LK_RANDOM_H
#define LK_RANDOM_H
#include "vm.h"
#include "fixnum.h"

/* type */
#define LK_RANDOM_N 624
#define LK_RANDOM(v) ((lk_random_t *)(v))
typedef struct lk_random {
    struct lk_common o;
    lk_fi_t          *seed;
    int               mti;
    unsigned long     mt[LK_RANDOM_N];
} lk_random_t;

/* ext map */
LK_LIB_DEFINEINIT(lk_random_extinit);
#endif
