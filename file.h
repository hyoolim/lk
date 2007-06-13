#ifndef LK_FILE_H
#define LK_FILE_H

/* type */
typedef struct lk_file lk_file_t;
#include "vm.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
struct lk_file {
    struct lk_common  co;
    lk_string_t      *path;
    union {
        FILE         *file;
        DIR          *dir;
    }                 st;
};
#define LK_FILE(v) ((lk_file_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_file_extinittypes);
LK_EXT_DEFINIT(lk_file_extinitfuncs);
#endif
