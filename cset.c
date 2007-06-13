#include "char.h"
#include "cset.h"
#include "ext.h"
#include "string.h"

/* ext map - types */
static LK_OBJECT_DEFALLOCFUNC(alloc__cset) {
    pt_cset_copy(CSET(self), CSET(proto));
}
static LK_OBJECT_DEFFREEFUNC(free__cset) {
    pt_cset_fin(CSET(self));
}
LK_EXT_DEFINIT(lk_cset_extinittypes) {
    vm->t_cset = lk_object_allocwithsize(vm->t_object, sizeof(lk_cset_t));
    pt_cset_init(CSET(vm->t_cset));
    lk_object_setallocfunc(vm->t_cset, alloc__cset);
    lk_object_setfreefunc(vm->t_cset, free__cset);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(addB__cset_cset) {
    pt_cset_addcset(CSET(self), CSET(ARG(0)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(addB__cset_str) {
    pt_cset_addstring(CSET(self), LIST(ARG(0)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(has__cset_ch) {
    RETURN(pt_cset_has(CSET(self), CHAR(ARG(0))) ? T : F);
}
static LK_EXT_DEFCFUNC(has__cset_str) {
    pt_list_t *str = LIST(ARG(0));
    PT_LIST_EACH(str, i, v, {
        if(!pt_cset_has(CSET(self), pt_list_getuchar(str, i))) RETURN(F);
    });
    RETURN(T);
}
static LK_EXT_DEFCFUNC(init__cset_str) {
    pt_cset_addstring(CSET(self), LIST(ARG(0)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(subtractB__cset_cset) {
    pt_cset_subtractcset(CSET(self), CSET(ARG(0)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(subtractB__cset_str) {
    pt_cset_subtractstring(CSET(self), LIST(ARG(0)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(to_s__cset) {
    lk_string_t *str = lk_string_new(VM);
    pt_list_t *s = LIST(str);
    uint32_t f, t;
    uint32_t *c = CSET(self)->data, *last = c + CSET(self)->count;
    if(CSET(self)->isinverted) pt_list_setuchar(s, s->count, '^');
    for(; c < last; ) {
        f = *c ++;
        t = *c ++;
        if(f == t) pt_list_setuchar(s, s->count, f);
        else {
            pt_list_setuchar(s, s->count, f);
            pt_list_setuchar(s, s->count, '-');
            pt_list_setuchar(s, s->count, t);
        }
    }
    RETURN(str);
}
static LK_EXT_DEFCFUNC(negateB__cset) {
    CSET(self)->isinverted = !CSET(self)->isinverted;
    RETURN(self);
}
LK_EXT_DEFINIT(lk_cset_extinitfuncs) {
    lk_object_t *cset = vm->t_cset, *ch = vm->t_char, *str = vm->t_string;
    lk_ext_global("CharacterSet", cset);
    lk_ext_cfunc(cset, "add!", addB__cset_cset, cset, NULL);
    lk_ext_cfunc(cset, "add!", addB__cset_str, str, NULL);
    lk_ext_cfunc(cset, "has?", has__cset_ch, ch, NULL);
    lk_ext_cfunc(cset, "has?", has__cset_str, str, NULL);
    lk_ext_cfunc(cset, "init", init__cset_str, str, NULL);
    lk_ext_cfunc(cset, "negate!", negateB__cset, NULL);
    lk_ext_cfunc(cset, "subtract!", subtractB__cset_cset, cset, NULL);
    lk_ext_cfunc(cset, "subtract!", subtractB__cset_str, str, NULL);
    lk_ext_cfunc(cset, "to_s", to_s__cset, NULL);
}

/* new */
lk_cset_t *lk_cset_new(lk_vm_t *vm) {
    return LK_CSET(lk_object_alloc(vm->t_cset));
}
