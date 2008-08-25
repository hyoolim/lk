#ifndef LK_BOOL_H
#define LK_BOOL_H
#include "vm.h"

/* type */
typedef lk_object_t lk_bool_t;

/* init */
void lk_bool_typeinit(lk_vm_t *vm);
void lk_bool_libinit(lk_vm_t *vm);
#endif
