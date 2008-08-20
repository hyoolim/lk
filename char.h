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
LK_LIB_DEFINEINIT(lk_char_libPreInit);
LK_LIB_DEFINEINIT(lk_char_libInit);

/* new */
lk_char_t *lk_char_new(lk_vm_t *vm, uint32_t c);
#endif
