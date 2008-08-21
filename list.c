#include "list.h"
#include "ext.h"
#include "number.h"

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark__list) {
    LIST_EACHPTR(DARRAY(self), i, v, mark(v));
}
LK_LIB_DEFINEINIT(lk_list_libPreInit) {
    vm->t_list = lk_object_alloc(vm->t_seq);
    darray_fin(DARRAY(vm->t_list));
    darray_init(DARRAY(vm->t_list), sizeof(lk_object_t *), 16);
    lk_object_setmarkfunc(vm->t_list, mark__list);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(at__darray_number) {
    lk_object_t *v = darray_getptr(DARRAY(self), CSIZE(ARG(0)));
    RETURN(v != NULL ? v : NIL);
}
LK_LIB_DEFINECFUNC(flatten__list) {
    lk_frame_t *caller = env->caller;
    if(!LIST_ISINIT(&caller->stack)) darray_initptr(&caller->stack);
    darray_concat(&caller->stack, DARRAY(self));
    DONE;
}
LK_LIB_DEFINECFUNC(insertB__darray_number_obj) {
    darray_insertptr(DARRAY(self), CSIZE(ARG(0)), lk_object_addref(self, ARG(0)));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(removeB__darray_number) {
    lk_object_t *v = darray_removeptr(DARRAY(self), CSIZE(ARG(0)));
    RETURN(v != NULL ? v : NIL);
}
LK_LIB_DEFINECFUNC(setB__darray_number_obj) {
    darray_setptr(DARRAY(self), CSIZE(ARG(0)), ARG(1));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(setB__darray_number_number_list) {
    darray_setrange(DARRAY(self), CSIZE(ARG(0)), CSIZE(ARG(1)), DARRAY(ARG(2)));
    RETURN(self);
}
LK_LIB_DEFINEINIT(lk_list_libInit) {
    lk_object_t *list = vm->t_list, *obj = vm->t_obj, *number = vm->t_number;
    lk_lib_setGlobal("List", list);
    lk_lib_setCFunc(list, "at", at__darray_number, number, NULL);
    lk_lib_setCFunc(list, "*", flatten__list, NULL);
    lk_lib_setCFunc(list, "insert!", insertB__darray_number_obj, number, obj, NULL);
    lk_lib_setCFunc(list, "remove!", removeB__darray_number, number, NULL);
    lk_lib_setCFunc(list, "set!", setB__darray_number_obj, number, obj, NULL);
    lk_lib_setCFunc(list, "set!", setB__darray_number_number_list, number, number, list, NULL);
}

/* new */
lk_list_t *lk_list_new(lk_vm_t *vm) {
    return LK_DARRAY(lk_object_alloc(vm->t_list));
}
lk_list_t *lk_list_newfromlist(lk_vm_t *vm, darray_t *from) {
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
