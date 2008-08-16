#include "error.h"
#include "ext.h"
#include "list.h"
#include <errno.h>

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(error__mark) {
    mark(LK_OBJ(LK_ERROR(self)->instr));
    mark(LK_OBJ(LK_ERROR(self)->text));
}
LK_EXT_DEFINIT(lk_error_extinittypes) {
    vm->t_error = lk_obj_allocwithsize(vm->t_obj, sizeof(lk_error_t));
    lk_obj_setmarkfunc(vm->t_error, error__mark);
}

/* ext map - funcs */
LK_EXT_DEFINIT(lk_error_extinitfuncs) {
    lk_obj_t *err = vm->t_error, *instr = vm->t_instr, *str = vm->t_string;
    lk_ext_global("Error", err);
    lk_ext_cfield(err, "instruction", instr, offsetof(lk_error_t, instr));
    lk_ext_cfield(err, "message", str, offsetof(lk_error_t, text));
    lk_ext_global("MessageError", LK_OBJ(lk_error_new(vm, vm->t_error, NULL)));
    lk_ext_global("NameError", LK_OBJ(lk_error_new(vm, vm->t_error, NULL)));
}

/* new */
lk_error_t *lk_error_new(lk_vm_t *vm, lk_obj_t *parent, const char *text) {
    lk_error_t *self = LK_ERROR(lk_obj_alloc(parent));
    self->instr = vm->currinstr;
    if(text != NULL) self->text = lk_string_newfromcstr(vm, text);
    return self;
}
