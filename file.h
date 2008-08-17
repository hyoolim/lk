#ifndef LK_FILE_H
#define LK_FILE_H

/* type */
typedef struct lk_File lk_File_t;
#define LK_FILE(v) ((lk_File_t *)(v))
#include "vm.h"
#include "string.h"
struct lk_File {
    struct lk_Common  obj;
    lk_String_t      *path;
    FILE             *file;
};

/* ext map */
LK_EXT_DEFINIT(lk_File_extinittypes);
LK_EXT_DEFINIT(lk_File_extinitfuncs);
#endif
