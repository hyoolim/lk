#include "str.h"
#include "char.h"
#include "charset.h"
#include "ext.h"
#include "num.h"

/* type */
void lk_str_typeinit(lk_vm_t *vm) {
    vm->t_str = lk_obj_alloc(vm->t_seq);
    darray_fin(DARRAY(vm->t_str));
    darray_init(DARRAY(vm->t_str), sizeof(uint8_t), 16);
}

/* new */
lk_str_t *lk_str_new(lk_vm_t *vm) {
    return LK_STRING(lk_obj_alloc(vm->t_str));
}
lk_str_t *lk_str_new_fromdarray(lk_vm_t *vm, darray_t *list) {
    lk_str_t *self = LK_STRING(lk_obj_alloc(vm->t_str));
    darray_copy(DARRAY(self), list);
    return self;
}
lk_str_t *lk_str_new_fromdata(lk_vm_t *vm, const void *data, int len) {
    darray_t *l = darray_alloc_fromdata(data, len);
    lk_str_t *s = lk_str_new_fromdarray(vm, l);
    darray_free(l);
    return s;
}
lk_str_t *lk_str_new_fromcstr(lk_vm_t *vm, const char *cstr) {
    return lk_str_new_fromdata(vm, cstr, strlen(cstr));
}

/* update */
void lk_str_set_at_char(lk_str_t *self, lk_num_t *at, lk_char_t *replacement) {
    darray_setuchar(DARRAY(self), CSIZE(at), CHAR(replacement));
}
void lk_str_set_range_str(lk_str_t *self, lk_num_t *from, lk_num_t *to, lk_str_t *replacement) {
    darray_t *x = DARRAY(self), *y = DARRAY(replacement);
    darray_resizeitem(x, y);
    darray_setrange(x, CSIZE(from), CSIZE(to), y);
}
void lk_str_unescape(lk_str_t *self) {
    darray_t *data = DARRAY(self);
    int i;
    uint32_t c;
    for(i = 0; (i = darray_find_char(data, '\\', i)) >= 0; i ++) {
        darray_removeuchar(data, i);
        switch(c = darray_getuchar(data, i + 1)) {
            case 'n': darray_setuchar(data, i, '\012'); break;
            case 'r': darray_setuchar(data, i, '\015'); break;
            case 't': darray_setuchar(data, i, '\t'  ); break;
            default : darray_setuchar(data, i, c     ); break;
        }
    }
}

/* info */
lk_char_t *lk_str_at(lk_str_t *self, lk_num_t *at) {
    return lk_char_new(VM, darray_getuchar(DARRAY(self), CSIZE(at)));
}
lk_obj_t *lk_str_find_char_starting(lk_str_t *self, lk_char_t *pattern, lk_num_t *starting) {
    int i = darray_find_char(DARRAY(self), CHAR(pattern), CSIZE(starting));
    return i >= 0 ? LK_OBJ(lk_num_new(VM, i)) : NIL;
}
lk_obj_t *lk_str_find_charset_starting(lk_str_t *self, lk_charset_t *pattern, lk_num_t *starting) {
    int i = darray_find_charset(DARRAY(self), CHARSET(pattern), CSIZE(starting));
    return i >= 0 ? LK_OBJ(lk_num_new(VM, i)) : NIL;
}
lk_obj_t *lk_str_find_str_starting(lk_str_t *self, lk_str_t *pattern, lk_num_t *starting) {
    int i = darray_find_darray(DARRAY(self), DARRAY(pattern), CSIZE(starting));
    return i >= 0 ? LK_OBJ(lk_num_new(VM, i)) : NIL;
}
lk_charset_t *lk_str_tocharset(lk_str_t *self) {
    lk_charset_t *charset = lk_charset_new(VM);
    charset_add_darray(CHARSET(charset), DARRAY(self));
    return charset;
}
lk_num_t *lk_str_tonum(lk_str_t *self) {
    numifn_t num;
    switch(num_new(0, DARRAY(self), &num)) {
        case NUMBERTYPE_INT: return lk_num_new(VM, num.i);
        case NUMBERTYPE_FLOAT: return lk_num_new(VM, num.f);
        default: BUG("Invalid num type while trying to parse code.\n");
    }
}

/* bind all c funcs to lk equiv */
void lk_str_libinit(lk_vm_t *vm) {
    lk_obj_t *str = vm->t_str, *num = vm->t_num, *charset = vm->t_charset, *ch = vm->t_char;
    lk_global_set("Newline", LK_OBJ(lk_str_new_fromcstr(vm, "\n")));
    lk_global_set("String", str);

    /* update */
    lk_obj_set_cfunc_cvoid(str, "set!", lk_str_set_at_char, num, ch, NULL);
    lk_obj_set_cfunc_cvoid(str, "set!", lk_str_set_range_str, num, num, str, NULL);

    /* info */
    lk_obj_set_cfunc_creturn(str, "at", lk_str_at, num, NULL);
    lk_obj_set_cfunc_creturn(str, "find", lk_str_find_char_starting, ch, num, NULL);
    lk_obj_set_cfunc_creturn(str, "find", lk_str_find_charset_starting, charset, num, NULL);
    lk_obj_set_cfunc_creturn(str, "find", lk_str_find_str_starting, str, num, NULL);
    lk_obj_set_cfunc_creturn(str, "toCharacterSet", lk_str_tocharset, NULL);
    lk_obj_set_cfunc_creturn(str, "toNumber", lk_str_tonum, NULL);
}
