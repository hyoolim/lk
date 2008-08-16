#ifndef LK_FIXNUM_H
#define LK_FIXNUM_H

/* type */
typedef struct lk_Fi lk_Fi_t;
typedef struct lk_Fr lk_Fr_t;
#include "vm.h"
struct lk_Fi {
    struct lk_Common obj;
    int              i;
};
#define LK_FI(v) ((lk_Fi_t *)(v))
struct lk_Fr {
    struct lk_Common obj;
    double           r;
};
#define LK_FR(v) ((lk_Fr_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Fixnum_extinittypes);
LK_EXT_DEFINIT(lk_Fixnum_extinitfuncs);

/* new */
lk_Fi_t *lk_Fi_new(lk_Vm_t *vm, int i);
lk_Fr_t *lk_Fr_new(lk_Vm_t *vm, double r);
#endif
