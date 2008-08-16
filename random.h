#ifndef LK_RANDOM_H
#define LK_RANDOM_H
#include "vm.h"
#include "fixnum.h"

/* type */
#define LK_RANDOM_N 624
typedef struct lk_Random {
    struct lk_Common  obj;
    lk_Fi_t          *seed;
    int               mti;
    unsigned long     mt[LK_RANDOM_N];
} lk_Random_t;
#define LK_RANDOM(v) ((lk_Random_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Random_extinit);
#endif
