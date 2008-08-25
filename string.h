#ifndef LK_STRING_H
#define LK_STRING_H
#include "types.h"

/* ext map */
void lk_string_typeinit(lk_vm_t *vm);
void lk_string_libinit(lk_vm_t *vm);

/* new */
lk_string_t *lk_string_new(lk_vm_t *vm);
lk_string_t *lk_string_newFromDArray(lk_vm_t *vm, darray_t *list);
lk_string_t *lk_string_newFromData(lk_vm_t *vm, const void *data, int len);
lk_string_t *lk_string_newFromCString(lk_vm_t *vm, const char *cstr);

/* update */
void lk_string_unescape(lk_string_t *self);
#endif
