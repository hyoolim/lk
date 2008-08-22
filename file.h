#ifndef LK_FILE_H
#define LK_FILE_H

/* type */
typedef struct lk_file lk_file_t;
#define LK_FILE(object) ((lk_file_t *)(object))
#include "vm.h"
#include "string.h"
struct lk_file {
    struct lk_common  o;
    lk_string_t      *path;
    FILE             *file;
};

/* init */
void lk_file_libPreInit(lk_vm_t *vm);
void lk_file_libInit(lk_vm_t *vm);
#endif
