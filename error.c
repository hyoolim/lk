#include "error.h"
#include "ext.h"
#include "list.h"
#include <errno.h>

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(error_mark) {
    mark(LK_OBJ(LK_ERROR(self)->instr));
    mark(LK_OBJ(LK_ERROR(self)->message));
}
void lk_error_libPreInit(lk_vm_t *vm) {
    vm->t_error = lk_object_allocWithSize(vm->t_object, sizeof(lk_error_t));
    lk_object_setmarkfunc(vm->t_error, error_mark);
}

/* ext map - funcs */
void lk_error_libInit(lk_vm_t *vm) {
    lk_object_t *err = vm->t_error, *instr = vm->t_instr, *str = vm->t_string;
    lk_lib_setGlobal("Error", err);
    lk_lib_setCField(err, "instruction", instr, offsetof(lk_error_t, instr));
    lk_lib_setCField(err, "message", str, offsetof(lk_error_t, message));
    lk_lib_setGlobal("MessageError", LK_OBJ(lk_error_new(vm, vm->t_error, NULL)));
    lk_lib_setGlobal("NameError", LK_OBJ(lk_error_new(vm, vm->t_error, NULL)));
}

/* new */
lk_error_t *lk_error_new(lk_vm_t *vm, lk_object_t *parent, const char *message) {
    lk_error_t *self = LK_ERROR(lk_object_alloc(parent));
    self->instr = vm->currinstr;
    if(message != NULL) self->message = lk_string_newFromCString(vm, message);
    return self;
}
