#include "char.h"
#include "cset.h"
#include "ext.h"
#include "string.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__cset) {
    CharacterSet_copy(CSET(self), CSET(parent));
}
static LK_OBJ_DEFFREEFUNC(free__cset) {
    CharacterSet_fin(CSET(self));
}
LK_EXT_DEFINIT(lk_Cset_extinittypes) {
    vm->t_cset = lk_Object_allocwithsize(vm->t_obj, sizeof(lk_Cset_t));
    CharacterSet_init(CSET(vm->t_cset));
    lk_Object_setallocfunc(vm->t_cset, alloc__cset);
    lk_Object_setfreefunc(vm->t_cset, free__cset);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(addB__CharacterSet_cset) {
    CharacterSet_add(CSET(self), CSET(ARG(0)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(addB__CharacterSet_str) {
    CharacterSet_addstring(CSET(self), LIST(ARG(0)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(has__CharacterSet_ch) {
    RETURN(CharacterSet_has(CSET(self), CHAR(ARG(0))) ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(has__CharacterSet_str) {
    Sequence_t *str = LIST(ARG(0));
    LIST_EACH(str, i, v, {
        if(!CharacterSet_has(CSET(self), Sequence_getuchar(str, i))) RETURN(F);
    });
    RETURN(T);
}
LK_LIBRARY_DEFINECFUNCTION(init__CharacterSet_str) {
    CharacterSet_addstring(CSET(self), LIST(ARG(0)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(subtractB__CharacterSet_cset) {
    CharacterSet_subtract(CSET(self), CSET(ARG(0)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(subtractB__CharacterSet_str) {
    CharacterSet_subtractstring(CSET(self), LIST(ARG(0)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(to_string__cset) {
    lk_String_t *str = lk_String_new(VM);
    Sequence_t *s = LIST(str);
    uint32_t f, t;
    uint32_t *c = CSET(self)->data, *last = c + CSET(self)->count;
    if(CSET(self)->isinverted) Sequence_setuchar(s, s->count, '^');
    for(; c < last; ) {
        f = *c ++;
        t = *c ++;
        if(f == t) Sequence_setuchar(s, s->count, f);
        else {
            Sequence_setuchar(s, s->count, f);
            Sequence_setuchar(s, s->count, '-');
            Sequence_setuchar(s, s->count, t);
        }
    }
    RETURN(str);
}
LK_LIBRARY_DEFINECFUNCTION(negateB__cset) {
    CSET(self)->isinverted = !CSET(self)->isinverted;
    RETURN(self);
}
LK_EXT_DEFINIT(lk_Cset_extinitfuncs) {
    lk_Object_t *cset = vm->t_cset, *ch = vm->t_char, *str = vm->t_string;
    lk_Library_setGlobal("CharacterSet", cset);
    lk_Library_setCFunction(cset, "+=", addB__CharacterSet_cset, cset, NULL);
    lk_Library_setCFunction(cset, "+=", addB__CharacterSet_str, str, NULL);
    lk_Library_setCFunction(cset, "has?", has__CharacterSet_ch, ch, NULL);
    lk_Library_setCFunction(cset, "has?", has__CharacterSet_str, str, NULL);
    lk_Library_setCFunction(cset, "init", init__CharacterSet_str, str, NULL);
    lk_Library_setCFunction(cset, "negate!", negateB__cset, NULL);
    lk_Library_setCFunction(cset, "-=", subtractB__CharacterSet_cset, cset, NULL);
    lk_Library_setCFunction(cset, "-=", subtractB__CharacterSet_str, str, NULL);
    lk_Library_setCFunction(cset, "toString", to_string__cset, NULL);
}

/* new */
lk_Cset_t *lk_Cset_new(lk_Vm_t *vm) {
    return LK_CSET(lk_Object_alloc(vm->t_cset));
}
