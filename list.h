#ifndef LK_LIST_H
#define LK_LIST_H

/* type */
typedef struct lk_glist lk_list_t;
#include "vm.h"
#include "glist.h"
#define LK_LIST(v) ((lk_list_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_list_extinittypes);
LK_EXT_DEFINIT(lk_list_extinitfuncs);

/* new */
lk_list_t *lk_list_new(lk_vm_t *vm);
lk_list_t *lk_list_newfromlist(lk_vm_t *vm, array_t *from);
lk_list_t *lk_list_newfromargv(lk_vm_t *vm, int argc, const char **argv);
#endif
