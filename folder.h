#ifndef LK_FOLDER_H
#define LK_FOLDER_H
#include "types.h"

/* type */
struct lk_folder {
    struct lk_common  o;
    lk_str_t      *path;
};

/* init */
void lk_folder_typeinit(lk_vm_t *vm);
void lk_folder_libinit(lk_vm_t *vm);

/* new */
void lk_folder_init(lk_folder_t *self, lk_str_t *path);

/* update */
void lk_folder_create(lk_folder_t *self);

/* info */
lk_list_t *lk_folder_items(lk_folder_t *self);
#endif
