#include "char.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__ch) {
    CHAR(self) = CHAR(proto);
}
LK_EXT_DEFINIT(lk_char_extinittypes) {
    vm->t_char = lk_obj_allocwithsize(vm->t_obj, sizeof(lk_char_t));
    lk_obj_setallocfunc(vm->t_char, alloc__ch);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(add__ch_fi) {
    if(INT(ARG(0)) > UINT32_MAX - CHAR(self)) {
        lk_vm_raisecstr(VM, "Will overflow");
    }
    RETURN(lk_char_new(VM, CHAR(self) + INT(ARG(0))));
}
static LK_EXT_DEFCFUNC(subtract__ch_ch) {
    RETURN(lk_fi_new(VM, CHAR(self) - CHAR(ARG(0))));
}
static LK_EXT_DEFCFUNC(subtract__ch_fi) {
    if(INT(ARG(0)) > CHAR(self)) {
        lk_vm_raisecstr(VM, "Will underflow");
    }
    RETURN(lk_char_new(VM, CHAR(self) - INT(ARG(0))));
}
static LK_EXT_DEFCFUNC(to_string__ch) {
    lk_string_t *str = lk_string_new(VM);
    list_setuchar(LIST(str), 0, CHAR(self));
    RETURN(str);
}
LK_EXT_DEFINIT(lk_char_extinitfuncs) {
    lk_obj_t *ch = vm->t_char, *fi = vm->t_fi;
    lk_ext_global("Character", ch);
    lk_ext_cfunc(ch, "+", add__ch_fi, fi, NULL);
    lk_ext_cfunc(ch, "<=>", subtract__ch_ch, ch, NULL);
    lk_ext_cfunc(ch, "-", subtract__ch_ch, ch, NULL);
    lk_ext_cfunc(ch, "-", subtract__ch_fi, fi, NULL);
    lk_ext_cfunc(ch, "to string", to_string__ch, NULL);
}

/* new */
lk_char_t *lk_char_new(lk_vm_t *vm, uint32_t c) {
    lk_char_t *self = LK_CHAR(lk_obj_alloc(vm->t_char));
    self->c = c;
    return self;
}
