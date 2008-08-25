#ifndef LK_LIST_H
#define LK_LIST_H
#include "types.h"

/* ext map */
void lk_list_typeinit(lk_vm_t *vm);
void lk_list_libinit(lk_vm_t *vm);

/* new */
lk_list_t *lk_list_new(lk_vm_t *vm);
lk_list_t *lk_list_newFromDArray(lk_vm_t *vm, darray_t *from);
lk_list_t *lk_list_newfromargv(lk_vm_t *vm, int argc, const char **argv);
#endif
