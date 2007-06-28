#include "error.h"
#include "ext.h"
#include "list.h"
#include <errno.h>

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(error__mark) {
    mark(LK_OBJ(LK_ERROR(self)->instr));
    mark(LK_OBJ(LK_ERROR(self)->text));
}
LK_VM_DEFGLOBAL(Bug);
LK_VM_DEFGLOBAL(SyntaxError);
LK_VM_DEFGLOBAL(MessageError);
LK_VM_DEFGLOBAL(NameError);
LK_VM_DEFGLOBAL(TypeError);
LK_VM_DEFGLOBAL(Error_C);
LK_EXT_DEFINIT(lk_error_extinittypes) {
    vm->t_error = lk_obj_allocwithsize(vm->t_obj, sizeof(lk_error_t));
    lk_obj_setmarkfunc(vm->t_error, error__mark);
    LK_VM_SETGLOBAL(vm, Bug, LK_OBJ(lk_error_new(vm, vm->t_error, NULL)));
    LK_VM_SETGLOBAL(vm, SyntaxError, LK_OBJ(lk_error_new(vm, vm->t_error, NULL)));
    LK_VM_SETGLOBAL(vm, MessageError, LK_OBJ(lk_error_new(vm, vm->t_error, NULL)));
    LK_VM_SETGLOBAL(vm, NameError, LK_OBJ(lk_error_new(vm, vm->t_error, NULL)));
    LK_VM_SETGLOBAL(vm, TypeError, LK_OBJ(lk_error_new(vm, vm->t_error, NULL)));
    LK_VM_SETGLOBAL(vm, Error_C, LK_OBJ(lk_error_new(vm, vm->t_error, NULL)));
}

/* ext map - funcs */
LK_EXT_DEFINIT(lk_error_extinitfuncs) {
    lk_obj_t *err = vm->t_error, *instr = vm->t_instr, *str = vm->t_string;
    lk_ext_global("Error", err);
    lk_ext_cfield(err, "instruction", instr, offsetof(lk_error_t, instr));
    lk_ext_cfield(err, "message", str, offsetof(lk_error_t, text));
    lk_ext_global("Bug", LK_VM_GETGLOBAL(vm, Bug));
    lk_ext_global("SyntaxError", LK_VM_GETGLOBAL(vm, SyntaxError));
    lk_ext_global("MessageError", LK_VM_GETGLOBAL(vm, MessageError));
    lk_ext_global("NameError", LK_VM_GETGLOBAL(vm, NameError));
    lk_ext_global("TypeError", LK_VM_GETGLOBAL(vm, TypeError));
    lk_ext_set(err, "C", LK_VM_GETGLOBAL(vm, Error_C));
}

/* new */
lk_error_t *lk_error_new(lk_vm_t *vm, lk_obj_t *proto, const char *text) {
    lk_error_t *self = LK_ERROR(lk_obj_alloc(proto));
    self->instr = vm->currinstr;
    if(text != NULL) self->text = lk_string_newfromcstr(vm, text);
    return self;
}
lk_error_t *lk_error_newc(lk_vm_t *vm) {
    return lk_error_new(vm, LK_VM_GETGLOBAL(vm, Error_C), strerror(errno));
}
lk_error_t *lk_error_newgenericerror(lk_vm_t *vm, const char *text) {
    return lk_error_new(vm, vm->t_error, text);
}
lk_error_t *lk_error_newsyntaxerror(lk_vm_t *vm, const char *text) {
    return lk_error_new(vm, LK_VM_GETGLOBAL(vm, SyntaxError), text);
}
lk_error_t *lk_error_newmessageerror(lk_vm_t *vm, const char *text, lk_string_t *msg, lk_obj_t *recv, lk_frame_t *args) {
    lk_error_t *self = lk_error_new(vm, LK_VM_GETGLOBAL(vm, MessageError), text);
    lk_obj_setslotbycstr(LK_OBJ(self), "message", vm->t_string, LK_OBJ(msg));
    lk_obj_setslotbycstr(LK_OBJ(self), "receiver", vm->t_obj, recv);
    lk_obj_setslotbycstr(LK_OBJ(self), "args", vm->t_obj,
    LK_OBJ(args != NULL ? lk_frame_stacktolist(args) : lk_list_new(vm)));
    return self;
}
lk_error_t *lk_error_newnameerror(lk_vm_t *vm, const char *text, lk_string_t *name) {
    lk_error_t *self = lk_error_new(vm, LK_VM_GETGLOBAL(vm, NameError), text);
    lk_obj_setslotbycstr(LK_OBJ(self), "name", vm->t_string, LK_OBJ(name));
    return self;
}
lk_error_t *lk_error_newtypeerror(lk_vm_t *vm, const char *text, lk_obj_t *given, lk_obj_t *expected) {
    lk_error_t *self = lk_error_new(vm, LK_VM_GETGLOBAL(vm, TypeError), text);
    lk_obj_setslotbycstr(LK_OBJ(self), "given", vm->t_obj, LK_OBJ(given));
    lk_obj_setslotbycstr(
    LK_OBJ(self), "expected", vm->t_obj, LK_OBJ(expected));
    return self;
}
