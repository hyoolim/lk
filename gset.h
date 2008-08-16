#ifndef LK_GSET_H
#define LK_GSET_H

/* type */
typedef struct lk_Gset lk_Gset_t;
#include "vm.h"
struct lk_Gset {
    struct lk_Common obj;
    set_t         set;
};
#define LK_GSET(v) ((lk_Gset_t *)(v))
#define SET(v) (&LK_GSET(v)->set)

/* ext map */
LK_EXT_DEFINIT(lk_Gset_extinittypes);
LK_EXT_DEFINIT(lk_Gset_extinitfuncs);
#endif
