#ifndef LK_FOLDER_H
#define LK_FOLDER_H

/* type */
typedef struct lk_Folder lk_Folder_t;
#define LK_FOLDER(self) ((lk_Folder_t *)(self))
#include "vm.h"
#include "string.h"
struct lk_Folder {
    struct lk_Common  obj;
    lk_String_t      *path;
};

/* ext map */
LK_EXT_DEFINIT(lk_Folder_extinittypes);
LK_EXT_DEFINIT(lk_Folder_extinitfuncs);
#endif
