#ifndef LK_FILE_H
#define LK_FILE_H
#include "types.h"

/* type */
struct lk_file {
    struct lk_common  o;
    lk_string_t      *path;
    FILE             *file;
};

/* init */
void lk_file_typeinit(lk_vm_t *vm);
void lk_file_libinit(lk_vm_t *vm);
#endif
