#include "char.h"
#include "charset.h"
#include "ext.h"
#include "string.h"

/* ext map - types */
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

/* ext map - funcs */
static void addB_charset_charset(lk_object_t *self, lk_scope_t *local) {
    charset_add_charset(CHARSET(self), CHARSET(ARG(0)));
    RETURN(self);
}
static void addB_charset_string(lk_object_t *self, lk_scope_t *local) {
    charset_add_darray(CHARSET(self), DARRAY(ARG(0)));
    RETURN(self);
}
static void has_charset_char(lk_object_t *self, lk_scope_t *local) {
    RETURN(charset_has(CHARSET(self), CHAR(ARG(0))) ? TRUE : FALSE);
}
static void has_charset_string(lk_object_t *self, lk_scope_t *local) {
    darray_t *str = DARRAY(ARG(0));
    LIST_EACH(str, i, v, {
        if(!charset_has(CHARSET(self), darray_getuchar(str, i))) RETURN(FALSE);
    });
    RETURN(TRUE);
}
static void init_charset_string(lk_object_t *self, lk_scope_t *local) {
    charset_add_darray(CHARSET(self), DARRAY(ARG(0)));
    RETURN(self);
}
static void subtractB_charset_charset(lk_object_t *self, lk_scope_t *local) {
    charset_subtract_charset(CHARSET(self), CHARSET(ARG(0)));
    RETURN(self);
}
static void subtractB_charset_string(lk_object_t *self, lk_scope_t *local) {
    charset_subtract_darray(CHARSET(self), DARRAY(ARG(0)));
    RETURN(self);
}
static void to_string_charset(lk_object_t *self, lk_scope_t *local) {
    darray_t *base = charset_tostring(CHARSET(self));
    lk_string_t *lk = lk_string_newFromDArray(VM, base);
    darray_free(base);
    RETURN(lk);
}
static void negateB_charset(lk_object_t *self, lk_scope_t *local) {
    charset_invert(CHARSET(self));
    RETURN(self);
}
void lk_charset_libinit(lk_vm_t *vm) {
    lk_object_t *charset = vm->t_charset, *ch = vm->t_char, *string = vm->t_string;
    lk_lib_setGlobal("CharacterSet", charset);
    lk_lib_setCFunc(charset, "+=", addB_charset_charset, charset, NULL);
    lk_lib_setCFunc(charset, "+=", addB_charset_string, string, NULL);
    lk_lib_setCFunc(charset, "has?", has_charset_char, ch, NULL);
    lk_lib_setCFunc(charset, "has?", has_charset_string, string, NULL);
    lk_lib_setCFunc(charset, "init!", init_charset_string, string, NULL);
    lk_lib_setCFunc(charset, "negate!", negateB_charset, NULL);
    lk_lib_setCFunc(charset, "-=", subtractB_charset_charset, charset, NULL);
    lk_lib_setCFunc(charset, "-=", subtractB_charset_string, string, NULL);
    lk_lib_setCFunc(charset, "toString", to_string_charset, NULL);
}

/* new */
lk_charset_t *lk_charset_new(lk_vm_t *vm) {
    return LK_CHARSET(lk_object_alloc(vm->t_charset));
}
