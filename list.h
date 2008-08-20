#ifndef LK_LIST_H
#define LK_LIST_H

/* type */
typedef struct lk_seq lk_list_t;
#define LK_DARRAY(v) ((lk_list_t *)(v))
#include "vm.h"
#include "seq.h"

/* ext map */
LK_LIB_DEFINEINIT(lk_list_libPreInit);
LK_LIB_DEFINEINIT(lk_list_libInit);

/* new */
lk_list_t *lk_list_new(lk_vm_t *vm);
lk_list_t *lk_list_newfromlist(lk_vm_t *vm, darray_t *from);
lk_list_t *lk_list_newfromargv(lk_vm_t *vm, int argc, const char **argv);
#endif
