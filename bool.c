#include "ext.h"

/* ext map - types */
void lk_bool_typeinit(lk_vm_t *vm) {
    vm->t_nil = lk_obj_alloc(vm->t_obj);
    vm->t_bool = lk_obj_alloc(vm->t_obj);
    vm->t_true = lk_obj_alloc(vm->t_bool);
    vm->t_false = lk_obj_alloc(vm->t_bool);
}

/* ext map - funcs */
void lk_bool_libinit(lk_vm_t *vm) {
    lk_lib_setGlobal("Nil", vm->t_nil);
    lk_lib_setGlobal("Boolean", vm->t_bool);
    lk_lib_setGlobal("True", vm->t_true);
    lk_lib_setGlobal("False", vm->t_false);
}
