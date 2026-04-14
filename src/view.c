#include "lib.h"

static LK_OBJ_DEFMARKFUNC(mark_view) {
    lk_view_t *tag = LK_VIEW(self);

    if ((ptrdiff_t)(tag->parent) & 1) {
        vec_t *parents = (vec_t *)((ptrdiff_t)(tag->parent) & ~1);
        VEC_EACH_PTR(parents, i, v, mark(v));
    } else if (tag->parent != NULL) {
        mark(tag->parent);
    }

    if (tag->ancestors != NULL)
        VEC_EACH_PTR(tag->ancestors, i, v, mark(v));

    if (tag->type != NULL)
        mark(tag->type);
}

static void free_view(lk_obj_t *self) {
    lk_view_t *tag = LK_VIEW(self);

    if ((ptrdiff_t)(tag->parent) & 1)
        vec_free((vec_t *)((ptrdiff_t)(tag->parent) & ~1));
    if (tag->ancestors != NULL)
        vec_free(tag->ancestors);
}

static void retrofit_view(lk_vm_t *vm, lk_view_t *tag) {
    tag->o.vm = vm;
    tag->o.view = LK_VIEW(vm->t_view);
    lk_objgroup_insert(vm->gc->permanent, LK_OBJ(tag));
}

void lk_view_type_init(lk_vm_t *vm) {
    lk_view_t *meta = mem_alloc(sizeof(lk_view_t));

    meta->o.vm = vm;
    meta->o.view = meta;
    meta->size = sizeof(lk_view_t);
    meta->mark_func = mark_view;
    meta->free_func = free_view;
    meta->parent = vm->t_obj;
    vm->t_view = LK_OBJ(meta);
    lk_objgroup_insert(vm->gc->permanent, LK_OBJ(meta));

    // Fix up the two tags allocated before the GC and t_view existed
    retrofit_view(vm, vm->t_obj->o.view);
    retrofit_view(vm, vm->gc->o.view);
}
