#include "bool.h"
#include "ext.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_bool_extinittypes) {
    vm->t_unknown = lk_object_alloc(vm->t_object);
    vm->t_bool = lk_object_alloc(vm->t_object);
    vm->t_true = lk_object_alloc(vm->t_bool);
    vm->t_false = lk_object_alloc(vm->t_bool);
}

/* ext map - funcs */
LK_EXT_DEFINIT(lk_bool_extinitfuncs) {
    lk_ext_global("Unknown", vm->t_unknown);
    lk_ext_global("Boolean", vm->t_bool);
    lk_ext_global("True", vm->t_true);
    lk_ext_global("False", vm->t_false);
}
