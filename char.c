#include "ext.h"

/* type */
static void alloc_ch(lk_object_t *self, lk_object_t *parent) {
    CHAR(self) = CHAR(parent);
}
void lk_char_typeinit(lk_vm_t *vm) {
    vm->t_char = lk_object_allocWithSize(vm->t_object, sizeof(lk_char_t));
    lk_object_setallocfunc(vm->t_char, alloc_ch);
}

/* new */
lk_char_t *lk_char_new(lk_vm_t *vm, uint32_t data) {
    lk_char_t *self = LK_CHAR(lk_object_alloc(vm->t_char));
    CHAR(self) = data;
    return self;
}

/* update */
void lk_char_add_number(lk_object_t *self, lk_number_t *other) {
    if(CNUMBER(other) > UINT32_MAX - CHAR(self)) {
        lk_vm_raisecstr(VM, "Will overflow");
    }
    CHAR(self) += CNUMBER(other);
}
void lk_char_subtract_char(lk_object_t *self, lk_char_t *other) {
    CHAR(self) -= CHAR(other);
}
void lk_char_subtract_number(lk_object_t *self, lk_number_t *other) {
    if(CNUMBER(other) > CHAR(self)) {
        lk_vm_raisecstr(VM, "Will underflow");
    }
    CHAR(self) -= CNUMBER(other);
}

/* info */
lk_number_t *lk_char_compare_char(lk_object_t *self, lk_char_t *other) {
    return lk_number_new(VM, CHAR(self) - CHAR(other));
}
lk_string_t *lk_char_tostring(lk_object_t *self) {
    lk_string_t *string = lk_string_new(VM);
    darray_setuchar(DARRAY(string), 0, CHAR(self));
    return string;
}

/* bind all c funcs to lk equiv */
void lk_char_libinit(lk_vm_t *vm) {
    lk_object_t *ch = vm->t_char, *number = vm->t_number;
    lk_lib_setGlobal("Character", ch);

    /* update */
    lk_object_set_cfunc_cvoid(ch, "+=", lk_char_add_number, number, NULL);
    lk_object_set_cfunc_cvoid(ch, "-=", lk_char_subtract_char, ch, NULL);
    lk_object_set_cfunc_cvoid(ch, "-=", lk_char_subtract_number, number, NULL);

    /* info */
    lk_object_set_cfunc_creturn(ch, "<=>", lk_char_compare_char, ch, NULL);
    lk_object_set_cfunc_creturn(ch, "toString", lk_char_tostring, NULL);
}
