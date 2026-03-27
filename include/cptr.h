#ifndef LK_CPTR_H
#define LK_CPTR_H
#include "types.h"

// type
typedef void lk_cptr_free_func_t(void *ptr);

struct lk_cptr {
    struct lk_common o;
    void *ptr;
    lk_cptr_free_func_t *free_func;
};
#define LK_CPTR(obj) ((lk_cptr_t *)(obj))

// init
void lk_cptr_type_init(lk_vm_t *vm);
void lk_cptr_lib_init(lk_vm_t *vm);

// new
lk_cptr_t *lk_cptr_new(lk_vm_t *vm, void *ptr, lk_cptr_free_func_t *free_func);
#endif
