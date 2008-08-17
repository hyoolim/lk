#include "char.h"
#include "charset.h"
#include "ext.h"
#include "string.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__charset) {
    charset_copy(CHARSET(self), CHARSET(parent));
}
static LK_OBJ_DEFFREEFUNC(free__charset) {
    charset_fin(CHARSET(self));
}
LK_EXT_DEFINIT(lk_charset_extinittypes) {
    vm->t_charset = lk_object_allocwithsize(vm->t_obj, sizeof(lk_charset_t));
    charset_init(CHARSET(vm->t_charset));
    lk_object_setallocfunc(vm->t_charset, alloc__charset);
    lk_object_setfreefunc(vm->t_charset, free__charset);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(addB__charset_charset) {
    charset_add(CHARSET(self), CHARSET(ARG(0)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(addB__charset_str) {
    charset_addArray(CHARSET(self), LIST(ARG(0)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(has__charset_ch) {
    RETURN(charset_has(CHARSET(self), CHAR(ARG(0))) ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(has__charset_str) {
    array_t *str = LIST(ARG(0));
    LIST_EACH(str, i, v, {
        if(!charset_has(CHARSET(self), array_getuchar(str, i))) RETURN(F);
    });
    RETURN(T);
}
LK_LIBRARY_DEFINECFUNCTION(init__charset_str) {
    charset_addArray(CHARSET(self), LIST(ARG(0)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(subtractB__charset_charset) {
    charset_subtract(CHARSET(self), CHARSET(ARG(0)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(subtractB__charset_str) {
    charset_subtractArray(CHARSET(self), LIST(ARG(0)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(to_string__charset) {
    lk_string_t *str = lk_string_new(VM);
    array_t *s = LIST(str);
    uint32_t f, t;
    uint32_t *c = CHARSET(self)->data, *last = c + CHARSET(self)->count;
    if(CHARSET(self)->isinverted) array_setuchar(s, s->count, '^');
    for(; c < last; ) {
        f = *c ++;
        t = *c ++;
        if(f == t) array_setuchar(s, s->count, f);
        else {
            array_setuchar(s, s->count, f);
            array_setuchar(s, s->count, '-');
            array_setuchar(s, s->count, t);
        }
    }
    RETURN(str);
}
LK_LIBRARY_DEFINECFUNCTION(negateB__charset) {
    CHARSET(self)->isinverted = !CHARSET(self)->isinverted;
    RETURN(self);
}
LK_EXT_DEFINIT(lk_charset_extinitfuncs) {
    lk_object_t *charset = vm->t_charset, *ch = vm->t_char, *str = vm->t_string;
    lk_library_setGlobal("CharacterSet", charset);
    lk_library_setCFunction(charset, "+=", addB__charset_charset, charset, NULL);
    lk_library_setCFunction(charset, "+=", addB__charset_str, str, NULL);
    lk_library_setCFunction(charset, "has?", has__charset_ch, ch, NULL);
    lk_library_setCFunction(charset, "has?", has__charset_str, str, NULL);
    lk_library_setCFunction(charset, "init", init__charset_str, str, NULL);
    lk_library_setCFunction(charset, "negate!", negateB__charset, NULL);
    lk_library_setCFunction(charset, "-=", subtractB__charset_charset, charset, NULL);
    lk_library_setCFunction(charset, "-=", subtractB__charset_str, str, NULL);
    lk_library_setCFunction(charset, "toString", to_string__charset, NULL);
}

/* new */
lk_charset_t *lk_charset_new(lk_vm_t *vm) {
    return LK_CHARSET(lk_object_alloc(vm->t_charset));
}
