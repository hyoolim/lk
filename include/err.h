#ifndef LK_ERROR_H
#define LK_ERROR_H
#include "types.h"

// init
void lk_err_type_init(lk_vm_t *vm);
void lk_err_lib_init(lk_vm_t *vm);

// new
lk_obj_t *lk_err_new(lk_vm_t *vm, lk_obj_t *parent, const char *message);
#endif
