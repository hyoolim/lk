#ifndef LK_CHARSET_H
#define LK_CHARSET_H

/* type */
typedef struct lk_charset lk_charset_t;
#define LK_CHARSET(object) ((lk_charset_t *)(object))
#include "vm.h"
#include "bool.h"
#include "char.h"
#include "string.h"
struct lk_charset {
    struct lk_common o;
    charset_t        data;
};

/* init */
void lk_charset_typeinit(lk_vm_t *vm);
void lk_charset_libinit(lk_vm_t *vm);

/* new */
lk_charset_t *lk_charset_new(lk_vm_t *vm);
void lk_charset_init_string(lk_object_t *self, lk_string_t *string);

/* update */
void lk_charset_add_charset(lk_object_t *self, lk_charset_t *other);
void lk_charset_add_string(lk_object_t *self, lk_string_t *string);
void lk_charset_negate(lk_object_t *self);
void lk_charset_subtract_charset(lk_object_t *self, lk_charset_t *other);
void lk_charset_subtract_string(lk_object_t *self, lk_string_t *other);

/* info */
lk_string_t *lk_charset_tostring(lk_object_t *self);
lk_bool_t *lk_charset_has_char(lk_object_t *self, lk_char_t *achar);
lk_bool_t *lk_charset_has_string(lk_object_t *self, lk_string_t *string);
#endif
