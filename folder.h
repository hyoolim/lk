#ifndef LK_FOLDER_H
#define LK_FOLDER_H

/* type */
typedef struct lk_folder lk_folder_t;
#define LK_FOLDER(object) ((lk_folder_t *)(object))
#include "vm.h"
#include "string.h"
struct lk_folder {
    struct lk_common  o;
    lk_string_t      *path;
};

/* init */
void lk_folder_typeinit(lk_vm_t *vm);
void lk_folder_libinit(lk_vm_t *vm);
#endif
