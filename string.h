#ifndef LK_STRING_H
#define LK_STRING_H

/* type */
typedef struct lk_Glist lk_String_t;
#include "vm.h"
#include "glist.h"
#define LK_STRING(v) ((lk_String_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_String_extinittypes);
LK_EXT_DEFINIT(lk_String_extinitfuncs);

/* new */
lk_String_t *lk_String_new(lk_Vm_t *vm);
lk_String_t *lk_String_newfromlist(lk_Vm_t *vm, Sequence_t *list);
lk_String_t *lk_String_newfromdata(lk_Vm_t *vm, const void *data, int len);
lk_String_t *lk_String_newfromcstr(lk_Vm_t *vm, const char *cstr);

/* update */
void lk_String_unescape(lk_String_t *self);
#endif
