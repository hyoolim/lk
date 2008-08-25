#include "ext.h"

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark_list) {
    LIST_EACHPTR(DARRAY(self), i, v, mark(v));
}
void lk_list_typeinit(lk_vm_t *vm) {
    vm->t_list = lk_object_alloc(vm->t_seq);
    darray_fin(DARRAY(vm->t_list));
    darray_init(DARRAY(vm->t_list), sizeof(lk_object_t *), 16);
    lk_object_setmarkfunc(vm->t_list, mark_list);
}

/* ext map - funcs */
static void at_darray_number(lk_object_t *self, lk_scope_t *local) {
    lk_object_t *v = darray_getptr(DARRAY(self), CSIZE(ARG(0)));
    RETURN(v != NULL ? v : NIL);
}
static void flatten_list(lk_object_t *self, lk_scope_t *local) {
    lk_scope_t *caller = local->caller;
    if(!LIST_ISINIT(&caller->stack)) darray_initptr(&caller->stack);
    darray_concat(&caller->stack, DARRAY(self));
    DONE;
}
static void insertB_darray_number_obj(lk_object_t *self, lk_scope_t *local) {
    darray_insertptr(DARRAY(self), CSIZE(ARG(0)), lk_object_addref(self, ARG(0)));
    RETURN(self);
}
static void removeB_darray_number(lk_object_t *self, lk_scope_t *local) {
    lk_object_t *v = darray_removeptr(DARRAY(self), CSIZE(ARG(0)));
    RETURN(v != NULL ? v : NIL);
}
static void setB_darray_number_obj(lk_object_t *self, lk_scope_t *local) {
    darray_setptr(DARRAY(self), CSIZE(ARG(0)), ARG(1));
    RETURN(self);
}
static void setB_darray_number_number_list(lk_object_t *self, lk_scope_t *local) {
    darray_setrange(DARRAY(self), CSIZE(ARG(0)), CSIZE(ARG(1)), DARRAY(ARG(2)));
    RETURN(self);
}
void lk_list_libinit(lk_vm_t *vm) {
    lk_object_t *list = vm->t_list, *obj = vm->t_object, *number = vm->t_number;
    lk_lib_setGlobal("List", list);
    lk_object_set_cfunc_lk(list, "at", at_darray_number, number, NULL);
    lk_object_set_cfunc_lk(list, "*", flatten_list, NULL);
    lk_object_set_cfunc_lk(list, "insert!", insertB_darray_number_obj, number, obj, NULL);
    lk_object_set_cfunc_lk(list, "remove!", removeB_darray_number, number, NULL);
    lk_object_set_cfunc_lk(list, "set!", setB_darray_number_obj, number, obj, NULL);
    lk_object_set_cfunc_lk(list, "set!", setB_darray_number_number_list, number, number, list, NULL);
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
