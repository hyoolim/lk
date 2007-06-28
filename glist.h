#ifndef LK_GLIST_H
#define LK_GLIST_H

/* type */
typedef struct lk_glist lk_glist_t;
#include "vm.h"
struct lk_glist {
    struct lk_common obj;
    list_t        data;
};
#define LK_GLIST(v) ((lk_glist_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_glist_extinittypes);
LK_EXT_DEFINIT(lk_glist_extinitfuncs);
#endif
