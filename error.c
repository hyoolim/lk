#include "error.h"
#include "ext.h"
#include "list.h"
#include <errno.h>

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(error__mark) {
    mark(LK_OBJ(LK_ERROR(self)->instr));
    mark(LK_OBJ(LK_ERROR(self)->text));
}
LK_EXT_DEFINIT(lk_Error_extinittypes) {
    vm->t_error = lk_Object_allocwithsize(vm->t_obj, sizeof(lk_Error_t));
    lk_Object_setmarkfunc(vm->t_error, error__mark);
}

/* ext map - funcs */
LK_EXT_DEFINIT(lk_Error_extinitfuncs) {
    lk_Object_t *err = vm->t_error, *instr = vm->t_instr, *str = vm->t_string;
    lk_Library_setGlobal("Error", err);
    lk_Library_cfield(err, "instruction", instr, offsetof(lk_Error_t, instr));
    lk_Library_cfield(err, "message", str, offsetof(lk_Error_t, text));
    lk_Library_setGlobal("MessageError", LK_OBJ(lk_Error_new(vm, vm->t_error, NULL)));
    lk_Library_setGlobal("NameError", LK_OBJ(lk_Error_new(vm, vm->t_error, NULL)));
}

/* new */
lk_Error_t *lk_Error_new(lk_Vm_t *vm, lk_Object_t *parent, const char *text) {
    lk_Error_t *self = LK_ERROR(lk_Object_alloc(parent));
    self->instr = vm->currinstr;
    if(text != NULL) self->text = lk_String_newfromcstr(vm, text);
    return self;
}
