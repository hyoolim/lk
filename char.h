#ifndef LK_CHAR_H
#define LK_CHAR_H

/* type */
typedef struct lk_char lk_char_t;
#define LK_CHAR(object) ((lk_char_t *)(object))
#include "vm.h"
struct lk_char {
    struct lk_common o;
    uint32_t         data;
};

/* init */
void lk_char_libPreInit(lk_vm_t *vm);
void lk_char_libInit(lk_vm_t *vm);

/* new */
lk_char_t *lk_char_new(lk_vm_t *vm, uint32_t data);
#endif
