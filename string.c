#include "string.h"
#include "char.h"
#include "charset.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_string_extinittypes) {
    vm->t_string = lk_object_alloc(vm->t_seq);
    array_fin(LIST(vm->t_string));
    array_init(LIST(vm->t_string), sizeof(uint8_t), 16);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(at__str_fi) {
    RETURN(lk_char_new(VM, array_getuchar(LIST(self), INT(ARG(0)))));
}
LK_LIBRARY_DEFINECFUNCTION(find__str_ch_fi) {
    int i = array_finduchar(LIST(self), CHAR(ARG(0)), INT(ARG(1)));
    if(i >= 0) RETURN(lk_fi_new(VM, i));
    RETURN(N);
}
LK_LIBRARY_DEFINECFUNCTION(find__str_charset_fi) {
    int i = array_findCharset(LIST(self), CHARSET(ARG(0)), INT(ARG(1)));
    if(i >= 0) RETURN(lk_fi_new(VM, i));
    RETURN(N);
}
LK_LIBRARY_DEFINECFUNCTION(find__str_str_fi) {
    int i = array_findlist(LIST(self), LIST(ARG(0)), INT(ARG(1)));
    if(i >= 0) RETURN(lk_fi_new(VM, i));
    RETURN(N);
}
LK_LIBRARY_DEFINECFUNCTION(setB__str_fi_ch) {
    array_setuchar(LIST(self), INT(ARG(0)), CHAR(ARG(1)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(setB__str_fi_fi_str) {
    array_t *x = LIST(self), *y = LIST(ARG(2));
    array_resizeitem(x, y);
    array_setrange(x, INT(ARG(0)), INT(ARG(1)), y);
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(to_character_set__str) {
    lk_charset_t *cs = lk_charset_new(VM);
    charset_addArray(CHARSET(cs), LIST(self));
    RETURN(cs);
}
LK_LIBRARY_DEFINECFUNCTION(to_number__str) {
    numberifn_t num;
    switch(number_new(0, LIST(self), &num)) {
    case NUMBERTYPE_INT: RETURN(lk_fi_new(VM, num.i));
    case NUMBERTYPE_FLOAT: RETURN(lk_fr_new(VM, num.f));
    default: BUG("Invalid number type while trying to parse code.\n");
    }
}
LK_EXT_DEFINIT(lk_string_extinitfuncs) {
    lk_object_t *str = vm->t_string, *fi = vm->t_fi, *charset = vm->t_charset,
                *ch = vm->t_char;
    lk_library_setGlobal("NEWLINE", LK_OBJ(lk_string_newfromcstr(vm, "\n")));
    lk_library_setGlobal("String", str);
    lk_library_setCFunction(str, "at", at__str_fi, fi, NULL);
    lk_library_setCFunction(str, "find", find__str_ch_fi, ch, fi, NULL);
    lk_library_setCFunction(str, "find", find__str_charset_fi, charset, fi, NULL);
    lk_library_setCFunction(str, "find", find__str_str_fi, str, fi, NULL);
    lk_library_setCFunction(str, "set!", setB__str_fi_ch, fi, ch, NULL);
    lk_library_setCFunction(str, "set!", setB__str_fi_fi_str, fi, fi, str, NULL);
    lk_library_setCFunction(str, "toCharacterSet", to_character_set__str, NULL);
    lk_library_setCFunction(str, "toNumber", to_number__str, NULL);
}

/* new */
lk_string_t *lk_string_new(lk_vm_t *vm) {
    return LK_STRING(lk_object_alloc(vm->t_string));
}
lk_string_t *lk_string_newfromlist(lk_vm_t *vm, array_t *list) {
    lk_string_t *self = LK_STRING(lk_object_alloc(vm->t_string));
    array_copy(LIST(self), list);
    return self;
}
lk_string_t *lk_string_newfromdata(lk_vm_t *vm, const void *data, int len) {
    array_t *l = array_allocFromData(data, len);
    lk_string_t *s = lk_string_newfromlist(vm, l);
    array_free(l);
    return s;
}
lk_string_t *lk_string_newfromcstr(lk_vm_t *vm, const char *cstr) {
    return lk_string_newfromdata(vm, cstr, strlen(cstr));
}

/* update */
#define UNESCAPE(c) do { \
    array_removeuchar(data, i); \
    array_setuchar(data, i, (c)); \
} while(0)
void lk_string_unescape(lk_string_t *self) {
    array_t *data = LIST(self);
    int i;
    uint32_t c;
    for(i = 0; (i = array_finduchar(data, '\\', i)) >= 0; i ++) {
        switch(c = array_getuchar(data, i + 1)) {
        case 'n': UNESCAPE('\012'); break;
        case 'r': UNESCAPE('\015'); break;
        case 't': UNESCAPE('\t'); break;
        default : UNESCAPE(c   ); break;
        }
    }
}
