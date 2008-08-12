#include "string.h"
#include "char.h"
#include "cset.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_string_extinittypes) {
    vm->t_string = lk_obj_alloc(vm->t_glist);
    list_fin(LIST(vm->t_string));
    list_init(LIST(vm->t_string), sizeof(uint8_t), 16);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(at__str_fi) {
    RETURN(lk_char_new(VM, list_getuchar(LIST(self), INT(ARG(0)))));
}
static LK_EXT_DEFCFUNC(find__str_ch_fi) {
    int i = list_finduchar(LIST(self), CHAR(ARG(0)), INT(ARG(1)));
    if(i >= 0) RETURN(lk_fi_new(VM, i));
    RETURN(N);
}
static LK_EXT_DEFCFUNC(find__str_cset_fi) {
    int i = list_findcset(LIST(self), CSET(ARG(0)), INT(ARG(1)));
    if(i >= 0) RETURN(lk_fi_new(VM, i));
    RETURN(N);
}
static LK_EXT_DEFCFUNC(find__str_str_fi) {
    int i = list_findlist(LIST(self), LIST(ARG(0)), INT(ARG(1)));
    if(i >= 0) RETURN(lk_fi_new(VM, i));
    RETURN(N);
}
static LK_EXT_DEFCFUNC(setB__str_fi_ch) {
    list_setuchar(LIST(self), INT(ARG(0)), CHAR(ARG(1)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(setB__str_fi_fi_str) {
    list_t *x = LIST(self), *y = LIST(ARG(2));
    list_resizeitem(x, y);
    list_setrange(x, INT(ARG(0)), INT(ARG(1)), y);
    RETURN(self);
}
static LK_EXT_DEFCFUNC(to_character_set__str) {
    lk_cset_t *cs = lk_cset_new(VM);
    cset_addstring(CSET(cs), LIST(self));
    RETURN(cs);
}
static LK_EXT_DEFCFUNC(to_number__str) {
    numberifn_t num;
    switch(number_new(0, LIST(self), &num)) {
    case NUMBERTYPE_INT: RETURN(lk_fi_new(VM, num.i));
    case NUMBERTYPE_FLOAT: RETURN(lk_fr_new(VM, num.f));
    default: BUG("Invalid number type while trying to parse code.\n");
    }
}
LK_EXT_DEFINIT(lk_string_extinitfuncs) {
    lk_obj_t *str = vm->t_string, *fi = vm->t_fi, *cset = vm->t_cset,
                *ch = vm->t_char;
    lk_ext_global("NEWLINE", LK_OBJ(lk_string_newfromcstr(vm, "\n")));
    lk_ext_global("String", str);
    lk_ext_cfunc(str, "at", at__str_fi, fi, NULL);
    lk_ext_cfunc(str, "find", find__str_ch_fi, ch, fi, NULL);
    lk_ext_cfunc(str, "find", find__str_cset_fi, cset, fi, NULL);
    lk_ext_cfunc(str, "find", find__str_str_fi, str, fi, NULL);
    lk_ext_cfunc(str, "set!", setB__str_fi_ch, fi, ch, NULL);
    lk_ext_cfunc(str, "set!", setB__str_fi_fi_str, fi, fi, str, NULL);
    lk_ext_cfunc(str, "toCharacterSet", to_character_set__str, NULL);
    lk_ext_cfunc(str, "toNumber", to_number__str, NULL);
}

/* new */
lk_string_t *lk_string_new(lk_vm_t *vm) {
    return LK_STRING(lk_obj_alloc(vm->t_string));
}
lk_string_t *lk_string_newfromlist(lk_vm_t *vm, list_t *list) {
    lk_string_t *self = LK_STRING(lk_obj_alloc(vm->t_string));
    list_copy(LIST(self), list);
    return self;
}
lk_string_t *lk_string_newfromdata(lk_vm_t *vm, const void *data, int len) {
    list_t *l = string_allocfromdata(data, len);
    lk_string_t *s = lk_string_newfromlist(vm, l);
    list_free(l);
    return s;
}
lk_string_t *lk_string_newfromcstr(lk_vm_t *vm, const char *cstr) {
    return lk_string_newfromdata(vm, cstr, strlen(cstr));
}

/* update */
#define UNESCAPE(c) do { \
    list_removeuchar(data, i); \
    list_setuchar(data, i, (c)); \
} while(0)
void lk_string_unescape(lk_string_t *self) {
    list_t *data = LIST(self);
    int i;
    uint32_t c;
    for(i = 0; (i = list_finduchar(data, '\\', i)) >= 0; i ++) {
        switch(c = list_getuchar(data, i + 1)) {
        case 'n': UNESCAPE('\012'); break;
        case 'r': UNESCAPE('\015'); break;
        case 't': UNESCAPE('\t'); break;
        default : UNESCAPE(c   ); break;
        }
    }
}
