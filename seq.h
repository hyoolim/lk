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
LK_EXT_DEFINIT(lk_seq_extinittypes);
LK_EXT_DEFINIT(lk_seq_extinitfuncs);
#endif