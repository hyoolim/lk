#include "string.h"
#include "char.h"
#include "charset.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_string_extinittypes) {
    vm->t_string = lk_object_alloc(vm->t_seq);
    darray_fin(DARRAY(vm->t_string));
    darray_init(DARRAY(vm->t_string), sizeof(uint8_t), 16);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(at__str_fi) {
    RETURN(lk_char_new(VM, darray_getuchar(DARRAY(self), INT(ARG(0)))));
}
LK_LIB_DEFINECFUNC(find__str_ch_fi) {
    int i = darray_findChar(DARRAY(self), CHAR(ARG(0)), INT(ARG(1)));
    if(i >= 0) RETURN(lk_fi_new(VM, i));
    RETURN(NIL);
}
LK_LIB_DEFINECFUNC(find__str_charset_fi) {
    int i = darray_findCharSet(DARRAY(self), CHARSET(ARG(0)), INT(ARG(1)));
    if(i >= 0) RETURN(lk_fi_new(VM, i));
    RETURN(NIL);
}
LK_LIB_DEFINECFUNC(find__str_str_fi) {
    int i = darray_findDArray(DARRAY(self), DARRAY(ARG(0)), INT(ARG(1)));
    if(i >= 0) RETURN(lk_fi_new(VM, i));
    RETURN(NIL);
}
LK_LIB_DEFINECFUNC(setB__str_fi_ch) {
    darray_setuchar(DARRAY(self), INT(ARG(0)), CHAR(ARG(1)));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(setB__str_fi_fi_str) {
    darray_t *x = DARRAY(self), *y = DARRAY(ARG(2));
    darray_resizeitem(x, y);
    darray_setrange(x, INT(ARG(0)), INT(ARG(1)), y);
    RETURN(self);
}
LK_LIB_DEFINECFUNC(to_character_qphash__str) {
    lk_charset_t *cs = lk_charset_new(VM);
    charset_addDArray(CHARSET(cs), DARRAY(self));
    RETURN(cs);
}
LK_LIB_DEFINECFUNC(to_number__str) {
    numberifn_t num;
    switch(number_new(0, DARRAY(self), &num)) {
    case NUMBERTYPE_INT: RETURN(lk_fi_new(VM, num.i));
    case NUMBERTYPE_FLOAT: RETURN(lk_fr_new(VM, num.f));
    default: BUG("Invalid number type while trying to parse code.\n");
    }
}
LK_EXT_DEFINIT(lk_string_extinitfuncs) {
    lk_object_t *str = vm->t_string, *fi = vm->t_fi, *charset = vm->t_charset,
                *ch = vm->t_char;
    lk_lib_setGlobal("NEWLINE", LK_OBJ(lk_string_newFromCString(vm, "\n")));
    lk_lib_setGlobal("String", str);
    lk_lib_setCFunc(str, "at", at__str_fi, fi, NULL);
    lk_lib_setCFunc(str, "find", find__str_ch_fi, ch, fi, NULL);
    lk_lib_setCFunc(str, "find", find__str_charset_fi, charset, fi, NULL);
    lk_lib_setCFunc(str, "find", find__str_str_fi, str, fi, NULL);
    lk_lib_setCFunc(str, "set!", setB__str_fi_ch, fi, ch, NULL);
    lk_lib_setCFunc(str, "set!", setB__str_fi_fi_str, fi, fi, str, NULL);
    lk_lib_setCFunc(str, "toCharacterSet", to_character_qphash__str, NULL);
    lk_lib_setCFunc(str, "toNumber", to_number__str, NULL);
}

/* new */
lk_string_t *lk_string_new(lk_vm_t *vm) {
    return LK_STRING(lk_object_alloc(vm->t_string));
}
lk_string_t *lk_string_newFromDArray(lk_vm_t *vm, darray_t *list) {
    lk_string_t *self = LK_STRING(lk_object_alloc(vm->t_string));
    darray_copy(DARRAY(self), list);
    return self;
}
lk_string_t *lk_string_newFromData(lk_vm_t *vm, const void *data, int len) {
    darray_t *l = darray_allocFromData(data, len);
    lk_string_t *s = lk_string_newFromDArray(vm, l);
    darray_free(l);
    return s;
}
lk_string_t *lk_string_newFromCString(lk_vm_t *vm, const char *cstr) {
    return lk_string_newFromData(vm, cstr, strlen(cstr));
}

/* update */
#define UNESCAPE(c) do { \
    darray_removeuchar(data, i); \
    darray_setuchar(data, i, (c)); \
} while(0)
void lk_string_unescape(lk_string_t *self) {
    darray_t *data = DARRAY(self);
    int i;
    uint32_t c;
    for(i = 0; (i = darray_findChar(data, '\\', i)) >= 0; i ++) {
        switch(c = darray_getuchar(data, i + 1)) {
        case 'n': UNESCAPE('\012'); break;
        case 'r': UNESCAPE('\015'); break;
        case 't': UNESCAPE('\t'); break;
        default : UNESCAPE(c   ); break;
        }
    }
}
