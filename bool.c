#include "bool.h"
#include "ext.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_Boolean_extinittypes) {
    vm->t_nil = lk_Object_alloc(vm->t_obj);
    vm->t_bool = lk_Object_alloc(vm->t_obj);
    vm->t_true = lk_Object_alloc(vm->t_bool);
    vm->t_false = lk_Object_alloc(vm->t_bool);
}

/* ext map - funcs */
LK_EXT_DEFINIT(lk_Boolean_extinitfuncs) {
    lk_Library_setGlobal("Nil", vm->t_nil);
    lk_Library_setGlobal("Boolean", vm->t_bool);
    lk_Library_setGlobal("True", vm->t_true);
    lk_Library_setGlobal("False", vm->t_false);
}
