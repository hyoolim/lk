#include "lib.h"
#include <errno.h>

// type
void lk_err_type_init(lk_vm_t *vm) {
    vm->t_err = lk_obj_alloc_type(vm->t_obj, sizeof(lk_obj_t));
}

// lib
void lk_err_lib_init(lk_vm_t *vm) {
    lk_global_set("Error", vm->t_err);
    lk_global_set("MessageError", lk_err_new(vm, vm->t_err, NULL));
    lk_global_set("NameError", lk_err_new(vm, vm->t_err, NULL));
}

// new
lk_obj_t *lk_err_new(lk_vm_t *vm, lk_obj_t *parent, const char *message) {
    lk_obj_t *self = lk_obj_alloc(parent);

    if (vm->currinstr != NULL)
        lk_obj_set_slot_by_cstr(self, "instr", NULL, LK_OBJ(vm->currinstr));

    if (message != NULL)
        lk_obj_set_slot_by_cstr(self, "message", NULL, LK_OBJ(lk_str_new_from_cstr(vm, message)));

    return self;
}
