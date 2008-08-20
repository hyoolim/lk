#ifndef LK_FIXNUM_H
#define LK_FIXNUM_H

/* type */
typedef struct lk_fi lk_fi_t;
typedef struct lk_fr lk_fr_t;
#define LK_FI(v) ((lk_fi_t *)(v))
#define LK_FR(v) ((lk_fr_t *)(v))
#include "vm.h"
struct lk_fi {
    struct lk_common o;
    int              data;
};
struct lk_fr {
    struct lk_common o;
    double           data;
};

/* ext map */
LK_LIB_DEFINEINIT(lk_fixnum_libPreInit);
LK_LIB_DEFINEINIT(lk_fixnum_libInit);

/* new */
lk_fi_t *lk_fi_new(lk_vm_t *vm, int i);
lk_fr_t *lk_fr_new(lk_vm_t *vm, double r);
#endif
