#ifndef LK_DIR_H
#define LK_DIR_H
#include "types.h"

// init
void lk_dir_type_init(lk_vm_t *vm);
void lk_dir_lib_init(lk_vm_t *vm);

// new
lk_obj_t *lk_dir_new_with_path(lk_vm_t *vm, lk_str_t *path);
void lk_dir_init(lk_obj_t *self, lk_str_t *path);

// update
void lk_dir_create(lk_obj_t *self);
void lk_dir_work(lk_obj_t *self);

// info
lk_list_t *lk_dir_items(lk_obj_t *self);
#endif
