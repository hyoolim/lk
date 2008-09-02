#include "lib.h"

/* type */
static LK_OBJ_DEFMARKFUNC(mark_list) {
    DARRAY_EACHPTR(DARRAY(self), i, v, mark(v));
}
void lk_list_typeinit(lk_vm_t *vm) {
    vm->t_list = lk_obj_alloc(vm->t_seq);
    darray_fin(DARRAY(vm->t_list));
    darray_init(DARRAY(vm->t_list), sizeof(lk_obj_t *), 16);
    lk_obj_setmarkfunc(vm->t_list, mark_list);
}

/* new */
lk_list_t *lk_list_new(lk_vm_t *vm) {
    return LK_DARRAY(lk_obj_alloc(vm->t_list));
}
lk_list_t *lk_list_new_fromdarray(lk_vm_t *vm, darray_t *from) {
    lk_list_t *self = lk_list_new(vm);
    darray_copy(DARRAY(self), from);
    return self;
}
lk_list_t *lk_list_newfromargv(lk_vm_t *vm, int argc, const char **argv) {
    lk_list_t *self = lk_list_new(vm);
    int i = 0;
    for(; i < argc; i ++) {
        darray_ptr_push(DARRAY(self), lk_str_new_fromcstr(vm, argv[i]));
    }
    return self;
}

/* update */
void lk_list_insert_num_obj(lk_list_t *self, lk_num_t *index, lk_obj_t *value) {
    darray_ptr_insert(DARRAY(self), CSIZE(index), lk_obj_addref(LK_OBJ(self), value));
}
void lk_list_remove_num(lk_list_t *self, lk_num_t *index) {
    darray_ptr_remove(DARRAY(self), CSIZE(index));
}
void lk_list_set_num_obj(lk_list_t *self, lk_num_t *index, lk_obj_t *value) {
    darray_ptr_set(DARRAY(self), CSIZE(index), value);
}
void lk_list_set_num_num_list(lk_list_t *self, lk_num_t *from, lk_num_t *to, lk_list_t *list) {
    darray_setrange(DARRAY(self), CSIZE(from), CSIZE(to), DARRAY(list));
}

/* info */
lk_obj_t *lk_list_at_num(lk_list_t *self, lk_num_t *index) {
    lk_obj_t *value = darray_ptr_get(DARRAY(self), CSIZE(index));
    return value != NULL ? value : NIL;
}
void lk_list_flatten(lk_obj_t *self, lk_scope_t *local) {
    lk_scope_t *caller = local->caller;
    if(!DARRAY_ISINIT(&caller->stack)) darray_ptr_init(&caller->stack);
    darray_concat(&caller->stack, DARRAY(self));
}

/* bind all c funcs to lk equiv */
void lk_list_libinit(lk_vm_t *vm) {
    lk_obj_t *list = vm->t_list, *obj = vm->t_obj, *num = vm->t_num;
    lk_global_set("List", list);

    /* update */
    lk_obj_set_cfunc_cvoid(list, "insert!", lk_list_insert_num_obj, num, obj, NULL);
    lk_obj_set_cfunc_cvoid(list, "remove!", lk_list_remove_num, num, NULL);
    lk_obj_set_cfunc_cvoid(list, "set!", lk_list_set_num_obj, num, obj, NULL);
    lk_obj_set_cfunc_cvoid(list, "set!", lk_list_set_num_num_list, num, num, list, NULL);

    /* info */
    lk_obj_set_cfunc_creturn(list, "at", lk_list_at_num, num, NULL);
    lk_obj_set_cfunc_lk(list, "*", lk_list_flatten, NULL);
}
