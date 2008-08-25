#include "ext.h"

/* init charset type */
static void alloc_charset(lk_object_t *self, lk_object_t *parent) {
    charset_copy(CHARSET(self), CHARSET(parent));
}
static void free_charset(lk_object_t *self) {
    charset_fin(CHARSET(self));
}
void lk_charset_typeinit(lk_vm_t *vm) {
    vm->t_charset = lk_object_allocWithSize(vm->t_object, sizeof(lk_charset_t));
    charset_init(CHARSET(vm->t_charset));
    lk_object_setallocfunc(vm->t_charset, alloc_charset);
    lk_object_setfreefunc(vm->t_charset, free_charset);
}

/* new */
lk_charset_t *lk_charset_new(lk_vm_t *vm) {
    return LK_CHARSET(lk_object_alloc(vm->t_charset));
}
void lk_charset_init_string(lk_object_t *self, lk_string_t *string) {
    charset_add_darray(CHARSET(self), DARRAY(string));
}

/* update */
void lk_charset_add_charset(lk_object_t *self, lk_charset_t *other) {
    charset_add_charset(CHARSET(self), CHARSET(other));
}
void lk_charset_add_string(lk_object_t *self, lk_string_t *string) {
    charset_add_darray(CHARSET(self), DARRAY(string));
}
void lk_charset_negate(lk_object_t *self) {
    charset_invert(CHARSET(self));
}
void lk_charset_subtract_charset(lk_object_t *self, lk_charset_t *other) {
    charset_subtract_charset(CHARSET(self), CHARSET(other));
}
void lk_charset_subtract_string(lk_object_t *self, lk_string_t *other) {
    charset_subtract_darray(CHARSET(self), DARRAY(other));
}

/* info */
lk_string_t *lk_charset_tostring(lk_object_t *self) {
    darray_t *base = charset_tostring(CHARSET(self));
    lk_string_t *lk = lk_string_newFromDArray(LK_VM(self), base);
    darray_free(base);
    return lk;
}
lk_bool_t *lk_charset_has_char(lk_object_t *self, lk_char_t *achar) {
    return charset_has(CHARSET(self), CHAR(achar)) ? TRUE : FALSE;
}
lk_bool_t *lk_charset_has_string(lk_object_t *self, lk_string_t *string) {
    darray_t *str = DARRAY(string);
    LIST_EACH(str, i, v, {
        if(!charset_has(CHARSET(self), darray_getuchar(str, i))) return FALSE;
    });
    return TRUE;
}

/* bind all c funcs to lk equiv */
void lk_charset_libinit(lk_vm_t *vm) {
    lk_object_t *charset = vm->t_charset, *ch = vm->t_char, *string = vm->t_string;
    lk_lib_setGlobal("CharacterSet", charset);
    lk_object_set_cfunc_cvoid(charset, "+=", lk_charset_add_charset, charset, NULL);
    lk_object_set_cfunc_cvoid(charset, "+=", lk_charset_add_string, string, NULL);
    lk_object_set_cfunc_creturn(charset, "has?", lk_charset_has_char, ch, NULL);
    lk_object_set_cfunc_creturn(charset, "has?", lk_charset_has_string, string, NULL);
    lk_object_set_cfunc_cvoid(charset, "init!", lk_charset_init_string, string, NULL);
    lk_object_set_cfunc_cvoid(charset, "negate!", lk_charset_negate, NULL);
    lk_object_set_cfunc_cvoid(charset, "-=", lk_charset_subtract_charset, charset, NULL);
    lk_object_set_cfunc_cvoid(charset, "-=", lk_charset_subtract_string, string, NULL);
    lk_object_set_cfunc_creturn(charset, "toString", lk_charset_tostring, NULL);
}
