#ifndef LK_GLIST_H
#define LK_GLIST_H

/* type */
typedef struct lk_seq lk_seq_t;
#define LK_GLIST(v) ((lk_seq_t *)(v))
#include "vm.h"
struct lk_seq {
    struct lk_common obj;
    array_t        data;
};

/* ext map */
LK_EXT_DEFINIT(lk_seq_extinittypes);
LK_EXT_DEFINIT(lk_seq_extinitfuncs);
#endif
