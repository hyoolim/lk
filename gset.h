#ifndef LK_GSET_H
#define LK_GSET_H

/* type */
typedef struct lk_gset lk_gset_t;
#include "vm.h"
struct lk_gset {
    struct lk_common obj;
    set_t         set;
};
#define LK_GSET(v) ((lk_gset_t *)(v))
#define SET(v) (&LK_GSET(v)->set)

/* ext map */
LK_EXT_DEFINIT(lk_gset_extinittypes);
LK_EXT_DEFINIT(lk_gset_extinitfuncs);
#endif
