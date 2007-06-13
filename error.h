#ifndef LK_ERROR_H
#define LK_ERROR_H

/* type */
typedef struct lk_error lk_error_t;
#include "vm.h"
#include "instr.h"
#include "file.h"
#include "string.h"
struct lk_error {
    struct lk_common  co;
    lk_instr_t       *instr;
    lk_string_t      *text;
};
#define LK_ERROR(v) ((lk_error_t *)(v))

/* ext map */
LK_VM_DEFGLOBAL_PROTO(Error);
LK_VM_DEFGLOBAL_PROTO(Bug);
LK_VM_DEFGLOBAL_PROTO(SyntaxError);
LK_VM_DEFGLOBAL_PROTO(MessageError);
LK_EXT_DEFINIT(lk_error_extinittypes);
LK_EXT_DEFINIT(lk_error_extinitfuncs);

/* new */
lk_error_t *lk_error_new(lk_vm_t *vm, lk_object_t *proto, const char *text);
lk_error_t *lk_error_newc(lk_vm_t *vm);
lk_error_t *lk_error_newgenericerror(lk_vm_t *vm, const char *text);
lk_error_t *lk_error_newsyntaxerror(lk_vm_t *vm, const char *text);
lk_error_t *lk_error_newmessageerror(lk_vm_t *vm, const char *text, lk_string_t *msg, lk_object_t *recv, lk_frame_t *args);
lk_error_t *lk_error_newnameerror(lk_vm_t *vm, const char *text, lk_string_t *name);
lk_error_t *lk_error_newtypeerror(lk_vm_t *vm, const char *text, lk_object_t *given, lk_object_t *expected);
#endif
