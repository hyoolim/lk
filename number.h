#ifndef LK_NUMBER_H
#define LK_NUMBER_H

/* type */
typedef struct lk_number lk_number_t;
#define LK_NUMBER(self) ((lk_number_t *)(self))
#include "vm.h"
struct lk_number {
    struct lk_common o;
    double           data;
};

/* ext map */
LK_LIB_DEFINEINIT(lk_number_libPreInit);
LK_LIB_DEFINEINIT(lk_number_libInit);

/* new */
lk_number_t *lk_number_new(lk_vm_t *vm, double number);
#endif
