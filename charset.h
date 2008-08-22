#ifndef LK_CHARSET_H
#define LK_CHARSET_H

/* type */
typedef struct lk_charset lk_charset_t;
#define LK_CHARSET(object) ((lk_charset_t *)(object))
#include "vm.h"
struct lk_charset {
    struct lk_common o;
    charset_t        data;
};

/* init */
LK_LIB_DEFINEINIT(lk_charset_libPreInit);
LK_LIB_DEFINEINIT(lk_charset_libInit);

/* new */
lk_charset_t *lk_charset_new(lk_vm_t *vm);
#endif
