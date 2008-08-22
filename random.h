#ifndef LK_RANDOM_H
#define LK_RANDOM_H
#include "vm.h"
#include "number.h"

/* type */
#define LK_RANDOM_N 624
#define LK_RANDOM(v) ((lk_random_t *)(v))
typedef struct lk_random {
    struct lk_common  o;
    lk_number_t      *seed;
    int               mti;
    unsigned long     mt[LK_RANDOM_N];
} lk_random_t;

/* ext map */
void lk_random_extinit(lk_vm_t *vm);
#endif
