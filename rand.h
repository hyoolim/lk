#ifndef LK_RANDOM_H
#define LK_RANDOM_H
#include "types.h"

/* type */
struct lk_rand {
    struct lk_common  o;
    lk_num_t      *seed;
    int               mti;
    unsigned long     mt[LK_RANDOM_N];
};

/* ext map */
void lk_rand_extinit(lk_vm_t *vm);
#endif
