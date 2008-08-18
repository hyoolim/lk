#ifndef LK_FILE_H
#define LK_FILE_H

/* type */
typedef struct lk_file lk_file_t;
#define LK_FILE(v) ((lk_file_t *)(v))
#include "vm.h"
#include "string.h"
struct lk_file {
    struct lk_common o;
    lk_string_t      *path;
    FILE             *file;
};

/* ext map */
LK_EXT_DEFINIT(lk_file_extinittypes);
LK_EXT_DEFINIT(lk_file_extinitfuncs);
#endif
