#ifndef LK_ERROR_H
#define LK_ERROR_H
#include "types.h"

/* type */
struct lk_err {
    struct lk_common  o;
    lk_instr_t       *instr;
    lk_str_t      *message;
};

/* init */
void lk_err_typeinit(lk_vm_t *vm);
void lk_err_libinit(lk_vm_t *vm);

/* new */
lk_err_t *lk_err_new(lk_vm_t *vm, lk_obj_t *parent, const char *message);
#endif
