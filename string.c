#include "string.h"
#include "char.h"
#include "cset.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_String_extinittypes) {
    vm->t_string = lk_Object_alloc(vm->t_glist);
    Sequence_fin(LIST(vm->t_string));
    Sequence_init(LIST(vm->t_string), sizeof(uint8_t), 16);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(at__str_fi) {
    RETURN(lk_Char_new(VM, Sequence_getuchar(LIST(self), INT(ARG(0)))));
}
LK_LIBRARY_DEFINECFUNCTION(find__str_ch_fi) {
    int i = Sequence_finduchar(LIST(self), CHAR(ARG(0)), INT(ARG(1)));
    if(i >= 0) RETURN(lk_Fi_new(VM, i));
    RETURN(N);
}
LK_LIBRARY_DEFINECFUNCTION(find__str_CharacterSet_fi) {
    int i = Sequence_findcset(LIST(self), CSET(ARG(0)), INT(ARG(1)));
    if(i >= 0) RETURN(lk_Fi_new(VM, i));
    RETURN(N);
}
LK_LIBRARY_DEFINECFUNCTION(find__str_str_fi) {
    int i = Sequence_findlist(LIST(self), LIST(ARG(0)), INT(ARG(1)));
    if(i >= 0) RETURN(lk_Fi_new(VM, i));
    RETURN(N);
}
LK_LIBRARY_DEFINECFUNCTION(setB__str_fi_ch) {
    Sequence_setuchar(LIST(self), INT(ARG(0)), CHAR(ARG(1)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(setB__str_fi_fi_str) {
    Sequence_t *x = LIST(self), *y = LIST(ARG(2));
    Sequence_resizeitem(x, y);
    Sequence_setrange(x, INT(ARG(0)), INT(ARG(1)), y);
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(to_character_set__str) {
    lk_Cset_t *cs = lk_Cset_new(VM);
    CharacterSet_addstring(CSET(cs), LIST(self));
    RETURN(cs);
}
LK_LIBRARY_DEFINECFUNCTION(to_number__str) {
    numberifn_t num;
    switch(number_new(0, LIST(self), &num)) {
    case NUMBERTYPE_INT: RETURN(lk_Fi_new(VM, num.i));
    case NUMBERTYPE_FLOAT: RETURN(lk_Fr_new(VM, num.f));
    default: BUG("Invalid number type while trying to parse code.\n");
    }
}
LK_EXT_DEFINIT(lk_String_extinitfuncs) {
    lk_Object_t *str = vm->t_string, *fi = vm->t_fi, *cset = vm->t_cset,
                *ch = vm->t_char;
    lk_Library_setGlobal("NEWLINE", LK_OBJ(lk_String_newfromcstr(vm, "\n")));
    lk_Library_setGlobal("String", str);
    lk_Library_setCFunction(str, "at", at__str_fi, fi, NULL);
    lk_Library_setCFunction(str, "find", find__str_ch_fi, ch, fi, NULL);
    lk_Library_setCFunction(str, "find", find__str_CharacterSet_fi, cset, fi, NULL);
    lk_Library_setCFunction(str, "find", find__str_str_fi, str, fi, NULL);
    lk_Library_setCFunction(str, "set!", setB__str_fi_ch, fi, ch, NULL);
    lk_Library_setCFunction(str, "set!", setB__str_fi_fi_str, fi, fi, str, NULL);
    lk_Library_setCFunction(str, "toCharacterSet", to_character_set__str, NULL);
    lk_Library_setCFunction(str, "toNumber", to_number__str, NULL);
}

/* new */
lk_String_t *lk_String_new(lk_Vm_t *vm) {
    return LK_STRING(lk_Object_alloc(vm->t_string));
}
lk_String_t *lk_String_newfromlist(lk_Vm_t *vm, Sequence_t *list) {
    lk_String_t *self = LK_STRING(lk_Object_alloc(vm->t_string));
    Sequence_copy(LIST(self), list);
    return self;
}
lk_String_t *lk_String_newfromdata(lk_Vm_t *vm, const void *data, int len) {
    Sequence_t *l = string_allocfromdata(data, len);
    lk_String_t *s = lk_String_newfromlist(vm, l);
    Sequence_free(l);
    return s;
}
lk_String_t *lk_String_newfromcstr(lk_Vm_t *vm, const char *cstr) {
    return lk_String_newfromdata(vm, cstr, strlen(cstr));
}

/* update */
#define UNESCAPE(c) do { \
    Sequence_removeuchar(data, i); \
    Sequence_setuchar(data, i, (c)); \
} while(0)
void lk_String_unescape(lk_String_t *self) {
    Sequence_t *data = LIST(self);
    int i;
    uint32_t c;
    for(i = 0; (i = Sequence_finduchar(data, '\\', i)) >= 0; i ++) {
        switch(c = Sequence_getuchar(data, i + 1)) {
        case 'n': UNESCAPE('\012'); break;
        case 'r': UNESCAPE('\015'); break;
        case 't': UNESCAPE('\t'); break;
        default : UNESCAPE(c   ); break;
        }
    }
}
