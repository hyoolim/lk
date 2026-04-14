#include "lib.h"

// ext map - types
void lk_bool_type_init(lk_vm_t *vm) {
    vm->t_nil = lk_obj_alloc_type(vm->t_obj, sizeof(lk_obj_t));
    vm->t_bool = lk_obj_alloc_type(vm->t_obj, sizeof(lk_obj_t));
    vm->t_true = lk_obj_alloc(vm->t_bool);
    vm->t_false = lk_obj_alloc(vm->t_bool);
}

// ext map - funcs
void lk_bool_lib_init(lk_vm_t *vm) {
    lk_global_set("Nil", vm->t_nil);
    lk_global_set("Boolean", vm->t_bool);
    lk_global_set("True", vm->t_true);
    lk_global_set("False", vm->t_false);
}
