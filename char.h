#ifndef LK_CHAR_H
#define LK_CHAR_H

/* type */
typedef struct lk_Char lk_Char_t;
#include "vm.h"
struct lk_Char {
    struct lk_Common obj;
    uint32_t         c;
};
#define LK_CHAR(v) ((lk_Char_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Char_extinittypes);
LK_EXT_DEFINIT(lk_Char_extinitfuncs);

/* new */
lk_Char_t *lk_Char_new(lk_Vm_t *vm, uint32_t c);
#endif
