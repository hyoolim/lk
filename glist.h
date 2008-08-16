#ifndef LK_GLIST_H
#define LK_GLIST_H

/* type */
typedef struct lk_Glist lk_GSequence_t;
#include "vm.h"
struct lk_Glist {
    struct lk_Common obj;
    Sequence_t        data;
};
#define LK_GLIST(v) ((lk_GSequence_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_GSequence_extinittypes);
LK_EXT_DEFINIT(lk_GSequence_extinitfuncs);
#endif
