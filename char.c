#include "char.h"
#include "ext.h"
#include "number.h"

/* ext map - types */
static void alloc_ch(lk_object_t *self, lk_object_t *parent) {
    CHAR(self) = CHAR(parent);
}
void lk_char_typeinit(lk_vm_t *vm) {
    vm->t_char = lk_object_allocWithSize(vm->t_object, sizeof(lk_char_t));
    lk_object_setallocfunc(vm->t_char, alloc_ch);
}

/* ext map - funcs */
static void add_ch_number(lk_object_t *self, lk_scope_t *local) {
    if(CNUMBER(ARG(0)) > UINT32_MAX - CHAR(self)) {
        lk_vm_raisecstr(VM, "Will overflow");
    }
    RETURN(lk_char_new(VM, CHAR(self) + CNUMBER(ARG(0))));
}
static void subtract_ch_ch(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_number_new(VM, CHAR(self) - CHAR(ARG(0))));
}
static void subtract_ch_number(lk_object_t *self, lk_scope_t *local) {
    if(CNUMBER(ARG(0)) > CHAR(self)) {
        lk_vm_raisecstr(VM, "Will underflow");
    }
    RETURN(lk_char_new(VM, CHAR(self) - CNUMBER(ARG(0))));
}
static void to_string_ch(lk_object_t *self, lk_scope_t *local) {
    lk_string_t *str = lk_string_new(VM);
    darray_setuchar(DARRAY(str), 0, CHAR(self));
    RETURN(str);
}
void lk_char_libinit(lk_vm_t *vm) {
    lk_object_t *ch = vm->t_char, *number = vm->t_number;
    lk_lib_setGlobal("Character", ch);
    lk_lib_setCFunc(ch, "+", add_ch_number, number, NULL);
    lk_lib_setCFunc(ch, "<=>", subtract_ch_ch, ch, NULL);
    lk_lib_setCFunc(ch, "-", subtract_ch_ch, ch, NULL);
    lk_lib_setCFunc(ch, "-", subtract_ch_number, number, NULL);
    lk_lib_setCFunc(ch, "toString", to_string_ch, NULL);
}

/* new */
lk_char_t *lk_char_new(lk_vm_t *vm, uint32_t data) {
    lk_char_t *self = LK_CHAR(lk_object_alloc(vm->t_char));
    CHAR(self) = data;
    return self;
}
