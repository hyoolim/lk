#ifndef LK_TAG_H
#define LK_TAG_H
#include "types.h"

// type
struct lk_tag {
    struct lk_common o;
    int refc;
    size_t size;
    lk_obj_t *parent;
    vec_t *ancestors;
    lk_tagallocfunc_t *alloc_func;
    lk_tagmarkfunc_t *mark_func;
    lk_tagfreefunc_t *free_func;
};
#define LK_TAG(obj) ((lk_tag_t *)(obj))

// init
void lk_tag_type_init(lk_vm_t *vm);
#endif
