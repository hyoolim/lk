#include "bool.h"
#include "ext.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_bool_extinittypes) {
    vm->t_nil = lk_obj_alloc(vm->t_obj);
    vm->t_bool = lk_obj_alloc(vm->t_obj);
    vm->t_true = lk_obj_alloc(vm->t_bool);
    vm->t_false = lk_obj_alloc(vm->t_bool);
}

/* ext map - funcs */
LK_EXT_DEFINIT(lk_bool_extinitfuncs) {
    lk_ext_global("Nil", vm->t_nil);
    lk_ext_global("Boolean", vm->t_bool);
    lk_ext_global("True", vm->t_true);
    lk_ext_global("False", vm->t_false);
}
