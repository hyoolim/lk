#include "bool.h"
#include "ext.h"

/* ext map - types */
LK_LIB_DEFINEINIT(lk_boolean_libPreInit) {
    vm->t_nil = lk_object_alloc(vm->t_obj);
    vm->t_bool = lk_object_alloc(vm->t_obj);
    vm->t_true = lk_object_alloc(vm->t_bool);
    vm->t_false = lk_object_alloc(vm->t_bool);
}

/* ext map - funcs */
LK_LIB_DEFINEINIT(lk_boolean_libInit) {
    lk_lib_setGlobal("Nil", vm->t_nil);
    lk_lib_setGlobal("Boolean", vm->t_bool);
    lk_lib_setGlobal("True", vm->t_true);
    lk_lib_setGlobal("False", vm->t_false);
}
