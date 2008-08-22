#include "char.h"
#include "charset.h"
#include "ext.h"
#include "string.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc_charset) {
    charset_copy(CHARSET(self), CHARSET(parent));
}
static LK_OBJ_DEFFREEFUNC(free_charset) {
    charset_fin(CHARSET(self));
}
LK_LIB_DEFINEINIT(lk_charset_libPreInit) {
    vm->t_charset = lk_object_allocWithSize(vm->t_object, sizeof(lk_charset_t));
    charset_init(CHARSET(vm->t_charset));
    lk_object_setallocfunc(vm->t_charset, alloc_charset);
    lk_object_setfreefunc(vm->t_charset, free_charset);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(addB_charset_charset) {
    charset_add_charset(CHARSET(self), CHARSET(ARG(0)));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(addB_charset_string) {
    charset_add_darray(CHARSET(self), DARRAY(ARG(0)));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(has_charset_char) {
    RETURN(charset_has(CHARSET(self), CHAR(ARG(0))) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(has_charset_string) {
    darray_t *str = DARRAY(ARG(0));
    LIST_EACH(str, i, v, {
        if(!charset_has(CHARSET(self), darray_getuchar(str, i))) RETURN(FALSE);
    });
    RETURN(TRUE);
}
LK_LIB_DEFINECFUNC(init_charset_string) {
    charset_add_darray(CHARSET(self), DARRAY(ARG(0)));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(subtractB_charset_charset) {
    charset_subtract_charset(CHARSET(self), CHARSET(ARG(0)));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(subtractB_charset_string) {
    charset_subtract_darray(CHARSET(self), DARRAY(ARG(0)));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(to_string_charset) {
    lk_string_t *str = lk_string_new(VM);
    /*
    darray_t *s = DARRAY(str);
    uint32_t f, t;
    uint32_t *c = CHARSET(self)->data, *last = c + CHARSET(self)->size;
    if(CHARSET_ISINVERTED(self)) darray_setuchar(s, s->size, '^');
    for(; c < last; ) {
        f = *c ++;
        t = *c ++;
        if(f == t) darray_setuchar(s, s->size, f);
        else {
            darray_setuchar(s, s->size, f);
            darray_setuchar(s, s->size, '-');
            darray_setuchar(s, s->size, t);
        }
    }
    */
    RETURN(str);
}
LK_LIB_DEFINECFUNC(negateB_charset) {
    charset_invert(CHARSET(self));
    RETURN(self);
}
LK_LIB_DEFINEINIT(lk_charset_libInit) {
    lk_object_t *charset = vm->t_charset, *ch = vm->t_char, *string = vm->t_string;
    lk_lib_setGlobal("CharacterSet", charset);
    lk_lib_setCFunc(charset, "+=", addB_charset_charset, charset, NULL);
    lk_lib_setCFunc(charset, "+=", addB_charset_string, string, NULL);
    lk_lib_setCFunc(charset, "has?", has_charset_char, ch, NULL);
    lk_lib_setCFunc(charset, "has?", has_charset_string, string, NULL);
    lk_lib_setCFunc(charset, "init!", init_charset_string, string, NULL);
    lk_lib_setCFunc(charset, "negate!", negateB_charset, NULL);
    lk_lib_setCFunc(charset, "-=", subtractB_charset_charset, charset, NULL);
    lk_lib_setCFunc(charset, "-=", subtractB_charset_string, string, NULL);
    lk_lib_setCFunc(charset, "toString", to_string_charset, NULL);
}

/* new */
lk_charset_t *lk_charset_new(lk_vm_t *vm) {
    return LK_CHARSET(lk_object_alloc(vm->t_charset));
}
