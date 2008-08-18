#ifndef LK_CHAR_H
#define LK_CHAR_H

/* type */
typedef struct lk_char lk_char_t;
#define LK_CHAR(v) ((lk_char_t *)(v))
#include "vm.h"
struct lk_char {
    struct lk_common o;
    uint32_t         data;
};

/* ext map */
LK_EXT_DEFINIT(lk_char_extinittypes);
LK_EXT_DEFINIT(lk_char_extinitfuncs);

/* new */
lk_char_t *lk_char_new(lk_vm_t *vm, uint32_t c);
#endif
