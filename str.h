#ifndef LK_STRING_H
#define LK_STRING_H
#include "types.h"

/* ext map */
void lk_str_typeinit(lk_vm_t *vm);
void lk_str_libinit(lk_vm_t *vm);

/* new */
lk_str_t *lk_str_new(lk_vm_t *vm);
lk_str_t *lk_str_new_fromdarray(lk_vm_t *vm, darray_t *list);
lk_str_t *lk_str_new_fromdata(lk_vm_t *vm, const void *data, int len);
lk_str_t *lk_str_new_fromcstr(lk_vm_t *vm, const char *cstr);

/* update */
void lk_str_unescape(lk_str_t *self);
#endif
