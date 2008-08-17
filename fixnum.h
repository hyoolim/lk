#ifndef LK_FIXNUM_H
#define LK_FIXNUM_H

/* type */
typedef struct lk_fi lk_fi_t;
typedef struct lk_fr lk_fr_t;
#define LK_FI(v) ((lk_fi_t *)(v))
#define LK_FR(v) ((lk_fr_t *)(v))
#include "vm.h"
struct lk_fi {
    struct lk_common obj;
    int              i;
};
struct lk_fr {
    struct lk_common obj;
    double           r;
};

/* ext map */
LK_EXT_DEFINIT(lk_fixnum_extinittypes);
LK_EXT_DEFINIT(lk_fixnum_extinitfuncs);

/* new */
lk_fi_t *lk_fi_new(lk_vm_t *vm, int i);
lk_fr_t *lk_fr_new(lk_vm_t *vm, double r);
#endif
