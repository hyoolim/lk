#ifndef LK_VIEW_H
#define LK_VIEW_H
#include "types.h"

// type
struct lk_view {
    struct lk_common o;
    size_t size;
    lk_obj_t *parent;
    vec_t *ancestors;
    lk_viewallocfunc_t *alloc_func;
    lk_viewmarkfunc_t *mark_func;
    lk_viewfreefunc_t *free_func;
};
#define LK_VIEW(obj) ((lk_view_t *)(obj))

// init
void lk_view_type_init(lk_vm_t *vm);
#endif
