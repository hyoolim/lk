#ifndef LK_CSET_H
#define LK_CSET_H

/* type */
typedef struct lk_charset lk_charset_t;
#include "vm.h"
struct lk_charset {
    struct lk_common obj;
    charset_t        cs;
};
#define LK_CSET(v) ((lk_charset_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_charset_extinittypes);
LK_EXT_DEFINIT(lk_charset_extinitfuncs);

/* new */
lk_charset_t *lk_charset_new(lk_vm_t *vm);
#endif
