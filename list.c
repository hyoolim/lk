#include "list.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark__list) {
    LIST_EACHPTR(LIST(self), i, v, mark(v));
}
LK_EXT_DEFINIT(lk_list_extinittypes) {
    vm->t_list = lk_object_alloc(vm->t_glist);
    array_fin(LIST(vm->t_list));
    array_init(LIST(vm->t_list), sizeof(lk_object_t *), 16);
    lk_object_setmarkfunc(vm->t_list, mark__list);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(at__array_fi) {
    lk_object_t *v = array_getptr(LIST(self), INT(ARG(0)));
    RETURN(v != NULL ? v : N);
}
LK_LIBRARY_DEFINECFUNCTION(flatten__list) {
    lk_frame_t *caller = env->caller;
    if(!LIST_ISINIT(&caller->stack)) array_initptr(&caller->stack);
    array_concat(&caller->stack, LIST(self));
    DONE;
}
LK_LIBRARY_DEFINECFUNCTION(insertB__array_fi_obj) {
    array_insertptr(LIST(self), INT(ARG(0)), lk_object_addref(self, ARG(0)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(removeB__array_fi) {
    lk_object_t *v = array_removeptr(LIST(self), INT(ARG(0)));
    RETURN(v != NULL ? v : N);
}
LK_LIBRARY_DEFINECFUNCTION(setB__array_fi_obj) {
    array_setptr(LIST(self), INT(ARG(0)), ARG(1));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(setB__array_fi_fi_list) {
    array_setrange(LIST(self), INT(ARG(0)), INT(ARG(1)), LIST(ARG(2)));
    RETURN(self);
}
LK_EXT_DEFINIT(lk_list_extinitfuncs) {
    lk_object_t *list = vm->t_list, *obj = vm->t_obj, *fi = vm->t_fi;
    lk_library_setGlobal("List", list);
    lk_library_setCFunction(list, "at", at__array_fi, fi, NULL);
    lk_library_setCFunction(list, "*", flatten__list, NULL);
    lk_library_setCFunction(list, "insert!", insertB__array_fi_obj, fi, obj, NULL);
    lk_library_setCFunction(list, "remove!", removeB__array_fi, fi, NULL);
    lk_library_setCFunction(list, "set!", setB__array_fi_obj, fi, obj, NULL);
    lk_library_setCFunction(list, "set!", setB__array_fi_fi_list, fi, fi, list, NULL);
}

/* new */
lk_list_t *lk_list_new(lk_vm_t *vm) {
    return LK_LIST(lk_object_alloc(vm->t_list));
}
lk_list_t *lk_list_newfromlist(lk_vm_t *vm, array_t *from) {
    lk_list_t *self = lk_list_new(vm);
    array_copy(LIST(self), from);
    return self;
}
lk_list_t *lk_list_newfromargv(lk_vm_t *vm, int argc, const char **argv) {
    lk_list_t *self = lk_list_new(vm);
    int i = 0;
    for(; i < argc; i ++) {
        array_pushptr(LIST(self), lk_string_newfromcstr(vm, argv[i]));
    }
    return self;
}
