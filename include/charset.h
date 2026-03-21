#ifndef LK_CHARSET_H
#define LK_CHARSET_H
#include "types.h"

/* type */
struct lk_charset {
    struct lk_common o;
    charset_t        data;
};

/* init */
void lk_charset_typeinit(lk_vm_t *vm);
void lk_charset_libinit(lk_vm_t *vm);

/* new */
lk_charset_t *lk_charset_new(lk_vm_t *vm);
void lk_charset_init_str(lk_obj_t *self, lk_str_t *str);

/* update */
void lk_charset_add_charset(lk_obj_t *self, lk_charset_t *other);
void lk_charset_add_str(lk_obj_t *self, lk_str_t *str);
void lk_charset_negate(lk_obj_t *self);
void lk_charset_subtract_charset(lk_obj_t *self, lk_charset_t *other);
void lk_charset_subtract_str(lk_obj_t *self, lk_str_t *other);

/* info */
lk_bool_t *lk_charset_has_char(lk_obj_t *self, lk_char_t *achar);
lk_bool_t *lk_charset_has_str(lk_obj_t *self, lk_str_t *str);
lk_str_t *lk_charset_tostr(lk_obj_t *self);
#endif
