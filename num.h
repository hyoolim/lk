#ifndef LK_NUMBER_H
#define LK_NUMBER_H
#include "types.h"

/* type */
struct lk_num {
    struct lk_common o;
    double           data;
};

/* ext map */
void lk_num_typeinit(lk_vm_t *vm);
void lk_num_libinit(lk_vm_t *vm);

/* new */
lk_num_t *lk_num_new(lk_vm_t *vm, double num);
#endif
