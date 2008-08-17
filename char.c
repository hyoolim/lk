#include "char.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__ch) {
    CHAR(self) = CHAR(parent);
}
LK_EXT_DEFINIT(lk_char_extinittypes) {
    vm->t_char = lk_object_allocwithsize(vm->t_obj, sizeof(lk_char_t));
    lk_object_setallocfunc(vm->t_char, alloc__ch);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(add__ch_fi) {
    if(INT(ARG(0)) > UINT32_MAX - CHAR(self)) {
        lk_vm_raisecstr(VM, "Will overflow");
    }
    RETURN(lk_char_new(VM, CHAR(self) + INT(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(subtract__ch_ch) {
    RETURN(lk_fi_new(VM, CHAR(self) - CHAR(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(subtract__ch_fi) {
    if(INT(ARG(0)) > CHAR(self)) {
        lk_vm_raisecstr(VM, "Will underflow");
    }
    RETURN(lk_char_new(VM, CHAR(self) - INT(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(to_string__ch) {
    lk_string_t *str = lk_string_new(VM);
    array_setuchar(LIST(str), 0, CHAR(self));
    RETURN(str);
}
LK_EXT_DEFINIT(lk_char_extinitfuncs) {
    lk_object_t *ch = vm->t_char, *fi = vm->t_fi;
    lk_library_setGlobal("Character", ch);
    lk_library_setCFunction(ch, "+", add__ch_fi, fi, NULL);
    lk_library_setCFunction(ch, "<=>", subtract__ch_ch, ch, NULL);
    lk_library_setCFunction(ch, "-", subtract__ch_ch, ch, NULL);
    lk_library_setCFunction(ch, "-", subtract__ch_fi, fi, NULL);
    lk_library_setCFunction(ch, "toString", to_string__ch, NULL);
}

/* new */
lk_char_t *lk_char_new(lk_vm_t *vm, uint32_t c) {
    lk_char_t *self = LK_CHAR(lk_object_alloc(vm->t_char));
    self->c = c;
    return self;
}
