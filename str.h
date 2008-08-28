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
void lk_str_set_at_char(lk_str_t *self, lk_num_t *at, lk_char_t *replacement);
void lk_str_set_range_str(lk_str_t *self, lk_num_t *from, lk_num_t *to, lk_str_t *replacement);
void lk_str_unescape(lk_str_t *self);

/* info */
lk_char_t *lk_str_at(lk_str_t *self, lk_num_t *at);
lk_obj_t *lk_str_find_char_starting(lk_str_t *self, lk_char_t *pattern, lk_num_t *starting);
lk_obj_t *lk_str_find_charset_starting(lk_str_t *self, lk_charset_t *pattern, lk_num_t *starting);
lk_obj_t *lk_str_find_str_starting(lk_str_t *self, lk_str_t *pattern, lk_num_t *starting);
lk_charset_t *lk_str_tocharset(lk_str_t *self);
lk_num_t *lk_str_tonum(lk_str_t *self);
#endif
