#ifndef LK_CHARSET_H
#define LK_CHARSET_H

/* type */
typedef struct lk_charset lk_charset_t;
#define LK_CHARSET(v) ((lk_charset_t *)(v))
#include "vm.h"
struct lk_charset {
    struct lk_common o;
    charset_t        data;
};

/* ext map */
LK_EXT_DEFINIT(lk_charset_extinittypes);
LK_EXT_DEFINIT(lk_charset_extinitfuncs);

/* new */
lk_charset_t *lk_charset_new(lk_vm_t *vm);
#endif
