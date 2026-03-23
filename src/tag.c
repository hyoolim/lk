#include "lib.h"

static LK_OBJ_DEFMARKFUNC(mark_tag) {
    lk_tag_t *tag = LK_TAG(self);

    if ((ptrdiff_t)(tag->parent) & 1) {
        vec_t *parents = (vec_t *)((ptrdiff_t)(tag->parent) & ~1);
        VEC_EACH_PTR(parents, i, v, mark(v));
    } else if (tag->parent != NULL) {
        mark(tag->parent);
    }

    if (tag->ancestors != NULL)
        VEC_EACH_PTR(tag->ancestors, i, v, mark(v));
}

static void free_tag(lk_obj_t *self) {
    lk_tag_t *tag = LK_TAG(self);

    if ((ptrdiff_t)(tag->parent) & 1)
        vec_free((vec_t *)((ptrdiff_t)(tag->parent) & ~1));
    if (tag->ancestors != NULL)
        vec_free(tag->ancestors);
}

static void retrofit_tag(lk_vm_t *vm, lk_tag_t *tag) {
    tag->o.vm = vm;
    tag->o.tag = LK_TAG(vm->t_tag);
    lk_objgroup_insert(vm->gc->permanent, LK_OBJ(tag));
}

void lk_tag_type_init(lk_vm_t *vm) {
    lk_tag_t *meta = mem_alloc(sizeof(lk_tag_t));

    meta->o.vm = vm;
    meta->o.tag = meta;
    meta->size = sizeof(lk_tag_t);
    meta->mark_func = mark_tag;
    meta->free_func = free_tag;
    meta->parent = vm->t_obj;
    vm->t_tag = LK_OBJ(meta);
    lk_objgroup_insert(vm->gc->permanent, LK_OBJ(meta));

    // Fix up the two tags allocated before the GC and t_tag existed
    retrofit_tag(vm, vm->t_obj->o.tag);
    retrofit_tag(vm, vm->gc->o.tag);
}
