#ifndef LK_FILE_H
#define LK_FILE_H

/* type */
typedef struct lk_File lk_File_t;
#include "vm.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
struct lk_File {
    struct lk_Common  obj;
    lk_String_t      *path;
    union {
        FILE         *file;
        DIR          *dir;
    }                 st;
};
#define LK_FILE(v) ((lk_File_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_File_extinittypes);
LK_EXT_DEFINIT(lk_File_extinitfuncs);
#endif
