#ifndef LK_LIST_H
#define LK_LIST_H

/* type */
typedef struct lk_Glist lk_List_t;
#include "vm.h"
#include "glist.h"
#define LK_LIST(v) ((lk_List_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_List_extinittypes);
LK_EXT_DEFINIT(lk_List_extinitfuncs);

/* new */
lk_List_t *lk_List_new(lk_Vm_t *vm);
lk_List_t *lk_List_newfromlist(lk_Vm_t *vm, Sequence_t *from);
lk_List_t *lk_List_newfromargv(lk_Vm_t *vm, int argc, const char **argv);
#endif
