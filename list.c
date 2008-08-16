#include "list.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark__list) {
    LIST_EACHPTR(LIST(self), i, v, mark(v));
}
LK_EXT_DEFINIT(lk_List_extinittypes) {
    vm->t_list = lk_Object_alloc(vm->t_glist);
    Sequence_fin(LIST(vm->t_list));
    Sequence_init(LIST(vm->t_list), sizeof(lk_Object_t *), 16);
    lk_Object_setmarkfunc(vm->t_list, mark__list);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(at__Sequence_fi) {
    lk_Object_t *v = Sequence_getptr(LIST(self), INT(ARG(0)));
    RETURN(v != NULL ? v : N);
}
LK_LIBRARY_DEFINECFUNCTION(flatten__list) {
    lk_Frame_t *caller = env->caller;
    if(!LIST_ISINIT(&caller->stack)) Sequence_initptr(&caller->stack);
    Sequence_concat(&caller->stack, LIST(self));
    DONE;
}
LK_LIBRARY_DEFINECFUNCTION(insertB__Sequence_fi_obj) {
    Sequence_insertptr(LIST(self), INT(ARG(0)), lk_Object_addref(self, ARG(0)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(removeB__Sequence_fi) {
    lk_Object_t *v = Sequence_removeptr(LIST(self), INT(ARG(0)));
    RETURN(v != NULL ? v : N);
}
LK_LIBRARY_DEFINECFUNCTION(setB__Sequence_fi_obj) {
    Sequence_setptr(LIST(self), INT(ARG(0)), ARG(1));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(setB__Sequence_fi_fi_list) {
    Sequence_setrange(LIST(self), INT(ARG(0)), INT(ARG(1)), LIST(ARG(2)));
    RETURN(self);
}
LK_EXT_DEFINIT(lk_List_extinitfuncs) {
    lk_Object_t *list = vm->t_list, *obj = vm->t_obj, *fi = vm->t_fi;
    lk_Library_setGlobal("List", list);
    lk_Library_setCFunction(list, "at", at__Sequence_fi, fi, NULL);
    lk_Library_setCFunction(list, "*", flatten__list, NULL);
    lk_Library_setCFunction(list, "insert!", insertB__Sequence_fi_obj, fi, obj, NULL);
    lk_Library_setCFunction(list, "remove!", removeB__Sequence_fi, fi, NULL);
    lk_Library_setCFunction(list, "set!", setB__Sequence_fi_obj, fi, obj, NULL);
    lk_Library_setCFunction(list, "set!", setB__Sequence_fi_fi_list, fi, fi, list, NULL);
}

/* new */
lk_List_t *lk_List_new(lk_Vm_t *vm) {
    return LK_LIST(lk_Object_alloc(vm->t_list));
}
lk_List_t *lk_List_newfromlist(lk_Vm_t *vm, Sequence_t *from) {
    lk_List_t *self = lk_List_new(vm);
    Sequence_copy(LIST(self), from);
    return self;
}
lk_List_t *lk_List_newfromargv(lk_Vm_t *vm, int argc, const char **argv) {
    lk_List_t *self = lk_List_new(vm);
    int i = 0;
    for(; i < argc; i ++) {
        Sequence_pushptr(LIST(self), lk_String_newfromcstr(vm, argv[i]));
    }
    return self;
}
