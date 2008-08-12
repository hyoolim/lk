#include "char.h"
#include "cset.h"
#include "ext.h"
#include "string.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__cset) {
    cset_copy(CSET(self), CSET(proto));
}
static LK_OBJ_DEFFREEFUNC(free__cset) {
    cset_fin(CSET(self));
}
LK_EXT_DEFINIT(lk_cset_extinittypes) {
    vm->t_cset = lk_obj_allocwithsize(vm->t_obj, sizeof(lk_cset_t));
    cset_init(CSET(vm->t_cset));
    lk_obj_setallocfunc(vm->t_cset, alloc__cset);
    lk_obj_setfreefunc(vm->t_cset, free__cset);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(addB__cset_cset) {
    cset_addcset(CSET(self), CSET(ARG(0)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(addB__cset_str) {
    cset_addstring(CSET(self), LIST(ARG(0)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(has__cset_ch) {
    RETURN(cset_has(CSET(self), CHAR(ARG(0))) ? T : F);
}
static LK_EXT_DEFCFUNC(has__cset_str) {
    list_t *str = LIST(ARG(0));
    LIST_EACH(str, i, v, {
        if(!cset_has(CSET(self), list_getuchar(str, i))) RETURN(F);
    });
    RETURN(T);
}
static LK_EXT_DEFCFUNC(init__cset_str) {
    cset_addstring(CSET(self), LIST(ARG(0)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(subtractB__cset_cset) {
    cset_subtractcset(CSET(self), CSET(ARG(0)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(subtractB__cset_str) {
    cset_subtractstring(CSET(self), LIST(ARG(0)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(to_string__cset) {
    lk_string_t *str = lk_string_new(VM);
    list_t *s = LIST(str);
    uint32_t f, t;
    uint32_t *c = CSET(self)->data, *last = c + CSET(self)->count;
    if(CSET(self)->isinverted) list_setuchar(s, s->count, '^');
    for(; c < last; ) {
        f = *c ++;
        t = *c ++;
        if(f == t) list_setuchar(s, s->count, f);
        else {
            list_setuchar(s, s->count, f);
            list_setuchar(s, s->count, '-');
            list_setuchar(s, s->count, t);
        }
    }
    RETURN(str);
}
static LK_EXT_DEFCFUNC(negateB__cset) {
    CSET(self)->isinverted = !CSET(self)->isinverted;
    RETURN(self);
}
LK_EXT_DEFINIT(lk_cset_extinitfuncs) {
    lk_obj_t *cset = vm->t_cset, *ch = vm->t_char, *str = vm->t_string;
    lk_ext_global("CharacterSet", cset);
    lk_ext_cfunc(cset, "+=", addB__cset_cset, cset, NULL);
    lk_ext_cfunc(cset, "+=", addB__cset_str, str, NULL);
    lk_ext_cfunc(cset, "has?", has__cset_ch, ch, NULL);
    lk_ext_cfunc(cset, "has?", has__cset_str, str, NULL);
    lk_ext_cfunc(cset, "init", init__cset_str, str, NULL);
    lk_ext_cfunc(cset, "negate!", negateB__cset, NULL);
    lk_ext_cfunc(cset, "-=", subtractB__cset_cset, cset, NULL);
    lk_ext_cfunc(cset, "-=", subtractB__cset_str, str, NULL);
    lk_ext_cfunc(cset, "toString", to_string__cset, NULL);
}

/* new */
lk_cset_t *lk_cset_new(lk_vm_t *vm) {
    return LK_CSET(lk_obj_alloc(vm->t_cset));
}
