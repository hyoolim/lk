#ifndef LK_CSET_H
#define LK_CSET_H

/* type */
typedef struct lk_Cset lk_Cset_t;
#include "vm.h"
struct lk_Cset {
    struct lk_Common obj;
    CharacterSet_t        cs;
};
#define LK_CSET(v) ((lk_Cset_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Cset_extinittypes);
LK_EXT_DEFINIT(lk_Cset_extinitfuncs);

/* new */
lk_Cset_t *lk_Cset_new(lk_Vm_t *vm);
#endif
