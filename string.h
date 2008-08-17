#ifndef LK_STRING_H
#define LK_STRING_H

/* type */
typedef struct lk_glist lk_string_t;
#include "vm.h"
#include "glist.h"
#define LK_STRING(v) ((lk_string_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_string_extinittypes);
LK_EXT_DEFINIT(lk_string_extinitfuncs);

/* new */
lk_string_t *lk_string_new(lk_vm_t *vm);
lk_string_t *lk_string_newfromlist(lk_vm_t *vm, array_t *list);
lk_string_t *lk_string_newfromdata(lk_vm_t *vm, const void *data, int len);
lk_string_t *lk_string_newfromcstr(lk_vm_t *vm, const char *cstr);

/* update */
void lk_string_unescape(lk_string_t *self);
#endif
