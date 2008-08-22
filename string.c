#include "string.h"
#include "char.h"
#include "charset.h"
#include "ext.h"
#include "number.h"

/* ext map - types */
void lk_string_libPreInit(lk_vm_t *vm) {
    vm->t_string = lk_object_alloc(vm->t_seq);
    darray_fin(DARRAY(vm->t_string));
    darray_init(DARRAY(vm->t_string), sizeof(uint8_t), 16);
}

/* ext map - funcs */
static void at_str_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_char_new(VM, darray_getuchar(DARRAY(self), CSIZE(ARG(0)))));
}
static void find_str_ch_number(lk_object_t *self, lk_scope_t *local) {
    int i = darray_findChar(DARRAY(self), CHAR(ARG(0)), CSIZE(ARG(1)));
    if(i >= 0) RETURN(lk_number_new(VM, i));
    RETURN(NIL);
}
static void find_str_charset_number(lk_object_t *self, lk_scope_t *local) {
    int i = darray_findCharSet(DARRAY(self), CHARSET(ARG(0)), CSIZE(ARG(1)));
    if(i >= 0) RETURN(lk_number_new(VM, i));
    RETURN(NIL);
}
static void find_str_str_number(lk_object_t *self, lk_scope_t *local) {
    int i = darray_findDArray(DARRAY(self), DARRAY(ARG(0)), CSIZE(ARG(1)));
    if(i >= 0) RETURN(lk_number_new(VM, i));
    RETURN(NIL);
}
static void setB_str_number_ch(lk_object_t *self, lk_scope_t *local) {
    darray_setuchar(DARRAY(self), CSIZE(ARG(0)), CHAR(ARG(1)));
    RETURN(self);
}
static void setB_str_number_number_str(lk_object_t *self, lk_scope_t *local) {
    darray_t *x = DARRAY(self), *y = DARRAY(ARG(2));
    darray_resizeitem(x, y);
    darray_setrange(x, CSIZE(ARG(0)), CSIZE(ARG(1)), y);
    RETURN(self);
}
static void to_character_qphash_str(lk_object_t *self, lk_scope_t *local) {
    lk_charset_t *cs = lk_charset_new(VM);
    charset_add_darray(CHARSET(cs), DARRAY(self));
    RETURN(cs);
}
static void to_number_str(lk_object_t *self, lk_scope_t *local) {
    numberifn_t num;
    switch(number_new(0, DARRAY(self), &num)) {
    case NUMBERTYPE_INT: RETURN(lk_number_new(VM, num.i));
    case NUMBERTYPE_FLOAT: RETURN(lk_number_new(VM, num.f));
    default: BUG("Invalid number type while trying to parse code.\n");
    }
}
void lk_string_libInit(lk_vm_t *vm) {
    lk_object_t *str = vm->t_string, *number = vm->t_number, *charset = vm->t_charset,
                *ch = vm->t_char;
    lk_lib_setGlobal("NEWLINE", LK_OBJ(lk_string_newFromCString(vm, "\n")));
    lk_lib_setGlobal("String", str);
    lk_lib_setCFunc(str, "at", at_str_number, number, NULL);
    lk_lib_setCFunc(str, "find", find_str_ch_number, ch, number, NULL);
    lk_lib_setCFunc(str, "find", find_str_charset_number, charset, number, NULL);
    lk_lib_setCFunc(str, "find", find_str_str_number, str, number, NULL);
    lk_lib_setCFunc(str, "set!", setB_str_number_ch, number, ch, NULL);
    lk_lib_setCFunc(str, "set!", setB_str_number_number_str, number, number, str, NULL);
    lk_lib_setCFunc(str, "toCharacterSet", to_character_qphash_str, NULL);
    lk_lib_setCFunc(str, "toNumber", to_number_str, NULL);
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
