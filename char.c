#include "char.h"
#include "ext.h"
#include "number.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__ch) {
    CHAR(self) = CHAR(parent);
}
LK_LIB_DEFINEINIT(lk_char_libPreInit) {
    vm->t_char = lk_object_allocwithsize(vm->t_obj, sizeof(lk_char_t));
    lk_object_setallocfunc(vm->t_char, alloc__ch);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(add__ch_number) {
    if(CNUMBER(ARG(0)) > UINT32_MAX - CHAR(self)) {
        lk_vm_raisecstr(VM, "Will overflow");
    }
    RETURN(lk_char_new(VM, CHAR(self) + CNUMBER(ARG(0))));
}
LK_LIB_DEFINECFUNC(subtract__ch_ch) {
    RETURN(lk_number_new(VM, CHAR(self) - CHAR(ARG(0))));
}
LK_LIB_DEFINECFUNC(subtract__ch_number) {
    if(CNUMBER(ARG(0)) > CHAR(self)) {
        lk_vm_raisecstr(VM, "Will underflow");
    }
    RETURN(lk_char_new(VM, CHAR(self) - CNUMBER(ARG(0))));
}
LK_LIB_DEFINECFUNC(to_string__ch) {
    lk_string_t *str = lk_string_new(VM);
    darray_setuchar(DARRAY(str), 0, CHAR(self));
    RETURN(str);
}
LK_LIB_DEFINEINIT(lk_char_libInit) {
    lk_object_t *ch = vm->t_char, *number = vm->t_number;
    lk_lib_setGlobal("Character", ch);
    lk_lib_setCFunc(ch, "+", add__ch_number, number, NULL);
    lk_lib_setCFunc(ch, "<=>", subtract__ch_ch, ch, NULL);
    lk_lib_setCFunc(ch, "-", subtract__ch_ch, ch, NULL);
    lk_lib_setCFunc(ch, "-", subtract__ch_number, number, NULL);
    lk_lib_setCFunc(ch, "toString", to_string__ch, NULL);
}

/* new */
lk_char_t *lk_char_new(lk_vm_t *vm, uint32_t c) {
    lk_char_t *self = LK_CHAR(lk_object_alloc(vm->t_char));
    CHAR(self) = c;
    return self;
}
