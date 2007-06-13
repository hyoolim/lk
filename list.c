#include "list.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
static LK_OBJECT_DEFMARKFUNC(mark__list) {
    PT_LIST_EACHPTR(LIST(self), i, v, mark(v));
}
LK_EXT_DEFINIT(lk_list_extinittypes) {
    vm->t_list = lk_object_alloc(vm->t_glist);
    pt_list_fin(LIST(vm->t_list));
    pt_list_init(LIST(vm->t_list), sizeof(lk_object_t *), 16);
    lk_object_setmarkfunc(vm->t_list, mark__list);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(at__list_fi) {
    lk_object_t *v = pt_list_getptr(LIST(self), INT(ARG(0)));
    RETURN(v != NULL ? v : N);
}
static LK_EXT_DEFCFUNC(flatten__list) {
    lk_frame_t *caller = env->caller;
    if(!PT_LIST_ISINIT(&caller->stack)) pt_list_initptr(&caller->stack);
    pt_list_concat(&caller->stack, LIST(self));
    DONE;
}
static LK_EXT_DEFCFUNC(insertB__list_fi_obj) {
    pt_list_insertptr(LIST(self), INT(ARG(0)), lk_object_addref(self, ARG(0)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(removeB__list_fi) {
    lk_object_t *v = pt_list_removeptr(LIST(self), INT(ARG(0)));
    RETURN(v != NULL ? v : N);
}
static LK_EXT_DEFCFUNC(setB__list_fi_obj) {
    pt_list_setptr(LIST(self), INT(ARG(0)), ARG(1));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(setB__list_fi_fi_list) {
    pt_list_setrange(LIST(self), INT(ARG(0)), INT(ARG(1)), LIST(ARG(2)));
    RETURN(self);
}
LK_EXT_DEFINIT(lk_list_extinitfuncs) {
    lk_object_t *list = vm->t_list, *obj = vm->t_object, *fi = vm->t_fi;
    lk_ext_global("List", list);
    lk_ext_cfunc(list, "at", at__list_fi, fi, NULL);
    lk_ext_cfunc(list, "*", flatten__list, NULL);
    lk_ext_cfunc(list, "insert!", insertB__list_fi_obj, fi, obj, NULL);
    lk_ext_cfunc(list, "remove!", removeB__list_fi, fi, NULL);
    lk_ext_cfunc(list, "set!", setB__list_fi_obj, fi, obj, NULL);
    lk_ext_cfunc(list, "set!", setB__list_fi_fi_list, fi, fi, list, NULL);
}

/* new */
lk_list_t *lk_list_new(lk_vm_t *vm) {
    return LK_LIST(lk_object_alloc(vm->t_list));
}
lk_list_t *lk_list_newfromlist(lk_vm_t *vm, pt_list_t *from) {
    lk_list_t *self = lk_list_new(vm);
    pt_list_copy(LIST(self), from);
    return self;
}
lk_list_t *lk_list_newfromargv(lk_vm_t *vm, int argc, const char **argv) {
    lk_list_t *self = lk_list_new(vm);
    int i = 0;
    for(; i < argc; i ++) {
        pt_list_pushptr(LIST(self), lk_string_newfromcstr(vm, argv[i]));
    }
    return self;
}
