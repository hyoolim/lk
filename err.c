#include "ext.h"
#include <errno.h>

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(err_mark) {
    mark(LK_OBJ(LK_ERROR(self)->instr));
    mark(LK_OBJ(LK_ERROR(self)->message));
}
void lk_err_typeinit(lk_vm_t *vm) {
    vm->t_err = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_err_t));
    lk_obj_setmarkfunc(vm->t_err, err_mark);
}

/* ext map - funcs */
void lk_err_libinit(lk_vm_t *vm) {
    lk_obj_t *err = vm->t_err, *instr = vm->t_instr, *str = vm->t_str;
    lk_global_set("Error", err);
    lk_obj_set_cfield(err, "instruction", instr, offsetof(lk_err_t, instr));
    lk_obj_set_cfield(err, "message", str, offsetof(lk_err_t, message));
    lk_global_set("MessageError", LK_OBJ(lk_err_new(vm, vm->t_err, NULL)));
    lk_global_set("NameError", LK_OBJ(lk_err_new(vm, vm->t_err, NULL)));
}

/* new */
lk_err_t *lk_err_new(lk_vm_t *vm, lk_obj_t *parent, const char *message) {
    lk_err_t *self = LK_ERROR(lk_obj_alloc(parent));
    self->instr = vm->currinstr;
    if(message != NULL) self->message = lk_str_new_fromcstr(vm, message);
    return self;
}
