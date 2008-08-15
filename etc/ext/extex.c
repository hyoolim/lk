#include "extex.h"
#include <lk/ext.h>

/* ext map */
LK_DEFCFUNC(add) {
    RETURN(lk_fixint_new(
    VM, LK_FIXINT(LK_EXTEX(SELF)->x)->i + LK_FIXINT(LK_EXTEX(SELF)->y)->i));
}
LK_DEFGLOBAL(ExtensionExample);
LK_DEFEXTINIT(lk_extex_extinit) {
    lk_object_t *t_extex, *t_fixint = LK_GETGLOBAL(vm, FixedInteger);
    t_extex = lk_ext_object(
    vm->t_number, sizeof(lk_extex_t), NULL, NULL, NULL);
    lk_ext_global("ExtensionExample", t_extex);
    LK_SETGLOBAL(vm, ExtensionExample, t_extex);
    lk_ext_cstructmember(t_extex, "x", t_fixint, offsetof(lk_extex_t, x));
    lk_ext_cstructmember(t_extex, "y", t_fixint, offsetof(lk_extex_t, y));
    lk_ext_cfuncwith0arg(t_extex, "+", add);
}
