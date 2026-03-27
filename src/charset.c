#include "lib.h"

// type
static void alloc_charset(lk_obj_t *self, lk_obj_t *parent) {
    charset_copy(CHARSET(self), CHARSET(parent));
}

static void free_charset(lk_obj_t *self) {
    charset_fin(CHARSET(self));
}

void lk_charset_type_init(lk_vm_t *vm) {
    vm->t_charset = lk_obj_alloc_type(vm->t_obj, sizeof(lk_charset_t));
    charset_init(CHARSET(vm->t_charset));
    lk_obj_set_alloc_func(vm->t_charset, alloc_charset);
    lk_obj_set_free_func(vm->t_charset, free_charset);
}

// new
lk_charset_t *lk_charset_new(lk_vm_t *vm) {
    return LK_CHARSET(lk_obj_alloc(vm->t_charset));
}

void lk_charset_init_str(lk_obj_t *self, lk_str_t *str) {
    charset_add_darray(CHARSET(self), VEC(str));
}

// update
void lk_charset_add_charset(lk_obj_t *self, lk_charset_t *other) {
    charset_add_charset(CHARSET(self), CHARSET(other));
}

void lk_charset_add_str(lk_obj_t *self, lk_str_t *str) {
    charset_add_darray(CHARSET(self), VEC(str));
}

void lk_charset_negate(lk_obj_t *self) {
    charset_invert(CHARSET(self));
}

void lk_charset_subtract_charset(lk_obj_t *self, lk_charset_t *other) {
    charset_subtract_charset(CHARSET(self), CHARSET(other));
}

void lk_charset_subtract_str(lk_obj_t *self, lk_str_t *other) {
    charset_subtract_darray(CHARSET(self), VEC(other));
}

// info
lk_bool_t *lk_charset_has_char(lk_obj_t *self, lk_char_t *achar) {
    return charset_has(CHARSET(self), CHAR(achar)) ? TRUE : FALSE;
}

lk_bool_t *lk_charset_has_str(lk_obj_t *self, lk_str_t *str) {
    vec_t *darray = VEC(str);
    VEC_EACH(darray, i, v, {
        (void)v;
        if (!charset_has(CHARSET(self), vec_str_get(darray, i)))
            return FALSE;
    });
    return TRUE;
}

lk_str_t *lk_charset_to_str(lk_obj_t *self) {
    vec_t *base = charset_tostr(CHARSET(self));
    lk_str_t *lk = lk_str_new_from_darray(LK_VM(self), base);
    vec_free(base);
    return lk;
}

// bind all c funcs to lk equiv
void lk_charset_lib_init(lk_vm_t *vm) {
    lk_obj_t *charset = vm->t_charset, *ch = vm->t_char, *str = vm->t_str;
    lk_global_set("CharacterSet", charset);

    // update
    lk_obj_set_cfunc_cvoid(charset, "+=", lk_charset_add_charset, charset, NULL);
    lk_obj_set_cfunc_cvoid(charset, "+=", lk_charset_add_str, str, NULL);
    lk_obj_set_cfunc_cvoid(charset, "init!", lk_charset_init_str, str, NULL);
    lk_obj_set_cfunc_cvoid(charset, "negate!", lk_charset_negate, NULL);
    lk_obj_set_cfunc_cvoid(charset, "-=", lk_charset_subtract_charset, charset, NULL);
    lk_obj_set_cfunc_cvoid(charset, "-=", lk_charset_subtract_str, str, NULL);

    // info
    lk_obj_set_cfunc_creturn(charset, "has?", lk_charset_has_char, ch, NULL);
    lk_obj_set_cfunc_creturn(charset, "has?", lk_charset_has_str, str, NULL);
    lk_obj_set_cfunc_creturn(charset, "toString", lk_charset_to_str, NULL);
}
