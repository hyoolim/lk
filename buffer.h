#ifndef LK_BUFFER_H
#define LK_BUFFER_H

/* type */
typedef struct lk_glist lk_buffer_t;
#include "vm.h"
#include "glist.h"
#define LK_BUFFER(v) ((lk_buffer_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_buffer_extinittypes);
LK_EXT_DEFINIT(lk_buffer_extinitfuncs);

/* new */
lk_buffer_t *lk_buffer_newfromlist(lk_vm_t *vm, list_t *buf);
#endif
