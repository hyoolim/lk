#ifndef LK_CHAR_H
#define LK_CHAR_H
#include "types.h"

// type
struct lk_char {
    struct lk_common o;
    uint32_t data;
};

// init
void lk_char_type_init(lk_vm_t *vm);
void lk_char_lib_init(lk_vm_t *vm);

// new
lk_char_t *lk_char_new(lk_vm_t *vm, uint32_t data);

// update
void lk_char_add_num(lk_obj_t *self, lk_num_t *other);
void lk_char_subtract_char(lk_obj_t *self, lk_char_t *other);
void lk_char_subtract_num(lk_obj_t *self, lk_num_t *other);

// info
lk_num_t *lk_char_compare_char(lk_obj_t *self, lk_char_t *other);
lk_str_t *lk_char_to_str(lk_obj_t *self);
#endif
