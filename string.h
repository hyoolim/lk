#ifndef LK_STRING_H
#define LK_STRING_H

/* type */
typedef struct lk_seq lk_string_t;
#define LK_STRING(v) ((lk_string_t *)(v))
#include "vm.h"
#include "seq.h"

/* ext map */
LK_EXT_DEFINIT(lk_string_extinittypes);
LK_EXT_DEFINIT(lk_string_extinitfuncs);

/* new */
lk_string_t *lk_string_new(lk_vm_t *vm);
lk_string_t *lk_string_newFromDArray(lk_vm_t *vm, darray_t *list);
lk_string_t *lk_string_newFromData(lk_vm_t *vm, const void *data, int len);
lk_string_t *lk_string_newFromCString(lk_vm_t *vm, const char *cstr);

/* update */
void lk_string_unescape(lk_string_t *self);
#endif
