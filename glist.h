#ifndef LK_GLIST_H
#define LK_GLIST_H

/* type */
typedef struct lk_glist lk_garray_t;
#include "vm.h"
struct lk_glist {
    struct lk_common obj;
    array_t        data;
};
#define LK_GLIST(v) ((lk_garray_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_garray_extinittypes);
LK_EXT_DEFINIT(lk_garray_extinitfuncs);
#endif
