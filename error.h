#ifndef LK_ERROR_H
#define LK_ERROR_H
#include "types.h"

/* type */
struct lk_error {
    struct lk_common  o;
    lk_instr_t       *instr;
    lk_string_t      *message;
};

/* init */
void lk_error_typeinit(lk_vm_t *vm);
void lk_error_libinit(lk_vm_t *vm);

/* new */
lk_error_t *lk_error_new(lk_vm_t *vm, lk_object_t *parent, const char *message);
#endif
