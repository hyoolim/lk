#ifndef LK_RANDOM_H
#define LK_RANDOM_H
#include "types.h"

/* type */
struct lk_random {
    struct lk_common  o;
    lk_number_t      *seed;
    int               mti;
    unsigned long     mt[LK_RANDOM_N];
};

/* ext map */
void lk_random_extinit(lk_vm_t *vm);
#endif
