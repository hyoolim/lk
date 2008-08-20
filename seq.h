#ifndef LK_SEQ_H
#define LK_SEQ_H

/* type */
typedef struct lk_seq lk_seq_t;
#define LK_SEQ(v) ((lk_seq_t *)(v))
#include "vm.h"
struct lk_seq {
    struct lk_common o;
    darray_t        data;
};

/* ext map */
LK_LIB_DEFINEINIT(lk_seq_libPreInit);
LK_LIB_DEFINEINIT(lk_seq_libInit);
#endif
