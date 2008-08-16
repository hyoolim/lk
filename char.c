#include "char.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__ch) {
    CHAR(self) = CHAR(parent);
}
LK_EXT_DEFINIT(lk_Char_extinittypes) {
    vm->t_char = lk_Object_allocwithsize(vm->t_obj, sizeof(lk_Char_t));
    lk_Object_setallocfunc(vm->t_char, alloc__ch);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(add__ch_fi) {
    if(INT(ARG(0)) > UINT32_MAX - CHAR(self)) {
        lk_Vm_raisecstr(VM, "Will overflow");
    }
    RETURN(lk_Char_new(VM, CHAR(self) + INT(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(subtract__ch_ch) {
    RETURN(lk_Fi_new(VM, CHAR(self) - CHAR(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(subtract__ch_fi) {
    if(INT(ARG(0)) > CHAR(self)) {
        lk_Vm_raisecstr(VM, "Will underflow");
    }
    RETURN(lk_Char_new(VM, CHAR(self) - INT(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(to_string__ch) {
    lk_String_t *str = lk_String_new(VM);
    Sequence_setuchar(LIST(str), 0, CHAR(self));
    RETURN(str);
}
LK_EXT_DEFINIT(lk_Char_extinitfuncs) {
    lk_Object_t *ch = vm->t_char, *fi = vm->t_fi;
    lk_Library_setGlobal("Character", ch);
    lk_Library_setCFunction(ch, "+", add__ch_fi, fi, NULL);
    lk_Library_setCFunction(ch, "<=>", subtract__ch_ch, ch, NULL);
    lk_Library_setCFunction(ch, "-", subtract__ch_ch, ch, NULL);
    lk_Library_setCFunction(ch, "-", subtract__ch_fi, fi, NULL);
    lk_Library_setCFunction(ch, "toString", to_string__ch, NULL);
}

/* new */
lk_Char_t *lk_Char_new(lk_Vm_t *vm, uint32_t c) {
    lk_Char_t *self = LK_CHAR(lk_Object_alloc(vm->t_char));
    self->c = c;
    return self;
}
