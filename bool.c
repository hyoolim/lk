#include "bool.h"
#include "ext.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_bool_extinittypes) {
    vm->t_unknown = lk_obj_alloc(vm->t_obj);
    vm->t_bool = lk_obj_alloc(vm->t_obj);
    vm->t_true = lk_obj_alloc(vm->t_bool);
    vm->t_false = lk_obj_alloc(vm->t_bool);
}

/* ext map - funcs */
LK_EXT_DEFINIT(lk_bool_extinitfuncs) {
    lk_ext_global("NIL", vm->t_unknown);
    lk_ext_global("NOT FOUND", vm->t_unknown);
    lk_ext_global("NULL", vm->t_unknown);
    lk_ext_global("UNKNOWN", vm->t_unknown);
    lk_ext_global("Boolean", vm->t_bool);
    lk_ext_global("TRUE", vm->t_true);
    lk_ext_global("FALSE", vm->t_false);
}
