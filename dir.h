#ifndef LK_DIR_H
#define LK_DIR_H
#include "types.h"

/* type */
struct lk_dir {
    struct lk_common  o;
    lk_str_t         *path;
    lk_str_t         *name;
};

/* init */
void lk_dir_typeinit(lk_vm_t *vm);
void lk_dir_libinit(lk_vm_t *vm);

/* new */
lk_dir_t *lk_dir_new_withpath(lk_vm_t *vm, lk_str_t *path);
void lk_dir_init(lk_dir_t *self, lk_str_t *path);

/* update */
void lk_dir_create(lk_dir_t *self);

/* info */
lk_list_t *lk_dir_items(lk_dir_t *self);
#endif
