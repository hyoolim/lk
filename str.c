#include "str.h"
#include "char.h"
#include "charset.h"
#include "ext.h"
#include "num.h"

/* ext map - types */
void lk_str_typeinit(lk_vm_t *vm) {
    vm->t_str = lk_obj_alloc(vm->t_seq);
    darray_fin(DARRAY(vm->t_str));
    darray_init(DARRAY(vm->t_str), sizeof(uint8_t), 16);
}

/* ext map - funcs */
static void at_str_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_char_new(VM, darray_getuchar(DARRAY(self), CSIZE(ARG(0)))));
}
static void find_str_ch_num(lk_obj_t *self, lk_scope_t *local) {
    int i = darray_findChar(DARRAY(self), CHAR(ARG(0)), CSIZE(ARG(1)));
    if(i >= 0) RETURN(lk_num_new(VM, i));
    RETURN(NIL);
}
static void find_str_charset_num(lk_obj_t *self, lk_scope_t *local) {
    int i = darray_findCharSet(DARRAY(self), CHARSET(ARG(0)), CSIZE(ARG(1)));
    if(i >= 0) RETURN(lk_num_new(VM, i));
    RETURN(NIL);
}
static void find_str_str_num(lk_obj_t *self, lk_scope_t *local) {
    int i = darray_findDArray(DARRAY(self), DARRAY(ARG(0)), CSIZE(ARG(1)));
    if(i >= 0) RETURN(lk_num_new(VM, i));
    RETURN(NIL);
}
static void setB_str_num_ch(lk_obj_t *self, lk_scope_t *local) {
    darray_setuchar(DARRAY(self), CSIZE(ARG(0)), CHAR(ARG(1)));
    RETURN(self);
}
static void setB_str_num_num_str(lk_obj_t *self, lk_scope_t *local) {
    darray_t *x = DARRAY(self), *y = DARRAY(ARG(2));
    darray_resizeitem(x, y);
    darray_setrange(x, CSIZE(ARG(0)), CSIZE(ARG(1)), y);
    RETURN(self);
}
static void to_character_qphash_str(lk_obj_t *self, lk_scope_t *local) {
    lk_charset_t *cs = lk_charset_new(VM);
    charset_add_darray(CHARSET(cs), DARRAY(self));
    RETURN(cs);
}
static void to_num_str(lk_obj_t *self, lk_scope_t *local) {
    numifn_t num;
    switch(num_new(0, DARRAY(self), &num)) {
    case NUMBERTYPE_INT: RETURN(lk_num_new(VM, num.i));
    case NUMBERTYPE_FLOAT: RETURN(lk_num_new(VM, num.f));
    default: BUG("Invalid num type while trying to parse code.\n");
    }
}
void lk_str_libinit(lk_vm_t *vm) {
    lk_obj_t *str = vm->t_str, *num = vm->t_num, *charset = vm->t_charset,
                *ch = vm->t_char;
    lk_lib_setGlobal("NEWLINE", LK_OBJ(lk_str_newFromCString(vm, "\n")));
    lk_lib_setGlobal("String", str);
    lk_obj_set_cfunc_lk(str, "at", at_str_num, num, NULL);
    lk_obj_set_cfunc_lk(str, "find", find_str_ch_num, ch, num, NULL);
    lk_obj_set_cfunc_lk(str, "find", find_str_charset_num, charset, num, NULL);
    lk_obj_set_cfunc_lk(str, "find", find_str_str_num, str, num, NULL);
    lk_obj_set_cfunc_lk(str, "set!", setB_str_num_ch, num, ch, NULL);
    lk_obj_set_cfunc_lk(str, "set!", setB_str_num_num_str, num, num, str, NULL);
    lk_obj_set_cfunc_lk(str, "toCharacterSet", to_character_qphash_str, NULL);
    lk_obj_set_cfunc_lk(str, "toNumber", to_num_str, NULL);
}

/* new */
lk_str_t *lk_str_new(lk_vm_t *vm) {
    return LK_STRING(lk_obj_alloc(vm->t_str));
}
lk_str_t *lk_str_newFromDArray(lk_vm_t *vm, darray_t *list) {
    lk_str_t *self = LK_STRING(lk_obj_alloc(vm->t_str));
    darray_copy(DARRAY(self), list);
    return self;
}
lk_str_t *lk_str_newFromData(lk_vm_t *vm, const void *data, int len) {
    darray_t *l = darray_allocFromData(data, len);
    lk_str_t *s = lk_str_newFromDArray(vm, l);
    darray_free(l);
    return s;
}
lk_str_t *lk_str_newFromCString(lk_vm_t *vm, const char *cstr) {
    return lk_str_newFromData(vm, cstr, strlen(cstr));
}

/* update */
#define UNESCAPE(c) do { \
    darray_removeuchar(data, i); \
    darray_setuchar(data, i, (c)); \
} while(0)
void lk_str_unescape(lk_str_t *self) {
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
