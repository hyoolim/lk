#include "bool.h"
#include "ext.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_boolean_extinittypes) {
    vm->t_nil = lk_object_alloc(vm->t_obj);
    vm->t_bool = lk_object_alloc(vm->t_obj);
    vm->t_true = lk_object_alloc(vm->t_bool);
    vm->t_false = lk_object_alloc(vm->t_bool);
}

/* ext map - funcs */
LK_EXT_DEFINIT(lk_boolean_extinitfuncs) {
    lk_library_setGlobal("Nil", vm->t_nil);
    lk_library_setGlobal("Boolean", vm->t_bool);
    lk_library_setGlobal("True", vm->t_true);
    lk_library_setGlobal("False", vm->t_false);
}
