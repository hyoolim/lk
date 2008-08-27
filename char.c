#include "ext.h"

/* type */
static void alloc_ch(lk_obj_t *self, lk_obj_t *parent) {
    CHAR(self) = CHAR(parent);
}
void lk_char_typeinit(lk_vm_t *vm) {
    vm->t_char = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_char_t));
    lk_obj_setallocfunc(vm->t_char, alloc_ch);
}

/* new */
lk_char_t *lk_char_new(lk_vm_t *vm, uint32_t data) {
    lk_char_t *self = LK_CHAR(lk_obj_alloc(vm->t_char));
    CHAR(self) = data;
    return self;
}

/* update */
void lk_char_add_num(lk_obj_t *self, lk_num_t *other) {
    if(CNUMBER(other) > UINT32_MAX - CHAR(self)) {
        lk_vm_raisecstr(VM, "Will overflow");
    }
    CHAR(self) += CNUMBER(other);
}
void lk_char_subtract_char(lk_obj_t *self, lk_char_t *other) {
    CHAR(self) -= CHAR(other);
}
void lk_char_subtract_num(lk_obj_t *self, lk_num_t *other) {
    if(CNUMBER(other) > CHAR(self)) {
        lk_vm_raisecstr(VM, "Will underflow");
    }
    CHAR(self) -= CNUMBER(other);
}

/* info */
lk_num_t *lk_char_compare_char(lk_obj_t *self, lk_char_t *other) {
    return lk_num_new(VM, CHAR(self) - CHAR(other));
}
lk_str_t *lk_char_tostr(lk_obj_t *self) {
    lk_str_t *str = lk_str_new(VM);
    darray_setuchar(DARRAY(str), 0, CHAR(self));
    return str;
}

/* bind all c funcs to lk equiv */
void lk_char_libinit(lk_vm_t *vm) {
    lk_obj_t *ch = vm->t_char, *num = vm->t_num;
    lk_global_set("Character", ch);

    /* update */
    lk_obj_set_cfunc_cvoid(ch, "+=", lk_char_add_num, num, NULL);
    lk_obj_set_cfunc_cvoid(ch, "-=", lk_char_subtract_char, ch, NULL);
    lk_obj_set_cfunc_cvoid(ch, "-=", lk_char_subtract_num, num, NULL);

    /* info */
    lk_obj_set_cfunc_creturn(ch, "<=>", lk_char_compare_char, ch, NULL);
    lk_obj_set_cfunc_creturn(ch, "toString", lk_char_tostr, NULL);
}
