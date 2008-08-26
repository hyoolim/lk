#include "ext.h"

/* type */
static LK_OBJ_DEFMARKFUNC(mark_list) {
    LIST_EACHPTR(DARRAY(self), i, v, mark(v));
}
void lk_list_typeinit(lk_vm_t *vm) {
    vm->t_list = lk_object_alloc(vm->t_seq);
    darray_fin(DARRAY(vm->t_list));
    darray_init(DARRAY(vm->t_list), sizeof(lk_object_t *), 16);
    lk_object_setmarkfunc(vm->t_list, mark_list);
}

/* new */
lk_list_t *lk_list_new(lk_vm_t *vm) {
    return LK_DARRAY(lk_object_alloc(vm->t_list));
}
lk_list_t *lk_list_newFromDArray(lk_vm_t *vm, darray_t *from) {
    lk_list_t *self = lk_list_new(vm);
    darray_copy(DARRAY(self), from);
    return self;
}
lk_list_t *lk_list_newfromargv(lk_vm_t *vm, int argc, const char **argv) {
    lk_list_t *self = lk_list_new(vm);
    int i = 0;
    for(; i < argc; i ++) {
        darray_pushptr(DARRAY(self), lk_string_newFromCString(vm, argv[i]));
    }
    return self;
}

/* update */
void lk_list_insert_number_object(lk_list_t *self, lk_number_t *index, lk_object_t *value) {
    darray_insertptr(DARRAY(self), CSIZE(index), lk_object_addref(LK_OBJ(self), value));
}
void lk_list_remove_number(lk_list_t *self, lk_number_t *index) {
    darray_removeptr(DARRAY(self), CSIZE(index));
}
void lk_list_set_number_object(lk_list_t *self, lk_number_t *index, lk_object_t *value) {
    darray_setptr(DARRAY(self), CSIZE(index), value);
}
void lk_list_set_number_number_list(lk_list_t *self, lk_number_t *from, lk_number_t *to, lk_list_t *list) {
    darray_setrange(DARRAY(self), CSIZE(from), CSIZE(to), DARRAY(list));
}

/* info */
lk_object_t *lk_list_at_number(lk_list_t *self, lk_number_t *index) {
    lk_object_t *value = darray_getptr(DARRAY(self), CSIZE(index));
    return value != NULL ? value : NIL;
}
void lk_list_flatten(lk_object_t *self, lk_scope_t *local) {
    lk_scope_t *caller = local->caller;
    if(!LIST_ISINIT(&caller->stack)) darray_initptr(&caller->stack);
    darray_concat(&caller->stack, DARRAY(self));
}

/* bind all c funcs to lk equiv */
void lk_list_libinit(lk_vm_t *vm) {
    lk_object_t *list = vm->t_list, *object = vm->t_object, *number = vm->t_number;
    lk_lib_setGlobal("List", list);

    /* update */
    lk_object_set_cfunc_cvoid(list, "insert!", lk_list_insert_number_object, number, object, NULL);
    lk_object_set_cfunc_cvoid(list, "remove!", lk_list_remove_number, number, NULL);
    lk_object_set_cfunc_cvoid(list, "set!", lk_list_set_number_object, number, object, NULL);
    lk_object_set_cfunc_cvoid(list, "set!", lk_list_set_number_number_list, number, number, list, NULL);

    /* info */
    lk_object_set_cfunc_creturn(list, "at", lk_list_at_number, number, NULL);
    lk_object_set_cfunc_lk(list, "*", lk_list_flatten, NULL);
}
