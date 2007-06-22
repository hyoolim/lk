#ifndef LK_CSET_H
#define LK_CSET_H

/* type */
typedef struct lk_cset lk_cset_t;
#include "vm.h"
struct lk_cset {
    struct lk_common co;
    cset_t        cs;
};
#define LK_CSET(v) ((lk_cset_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_cset_extinittypes);
LK_EXT_DEFINIT(lk_cset_extinitfuncs);

/* new */
lk_cset_t *lk_cset_new(lk_vm_t *vm);
#endif
