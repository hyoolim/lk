#ifndef LK_CHAR_H
#define LK_CHAR_H
#include "types.h"

/* type */
struct lk_char {
    struct lk_common o;
    uint32_t         data;
};

/* init */
void lk_char_typeinit(lk_vm_t *vm);
void lk_char_libinit(lk_vm_t *vm);

/* new */
lk_char_t *lk_char_new(lk_vm_t *vm, uint32_t data);
#endif
