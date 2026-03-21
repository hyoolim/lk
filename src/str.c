#include "str.h"
#include "char.h"
#include "charset.h"
#include "lib.h"
#include "num.h"

// type
void lk_str_type_init(lk_vm_t *vm) {
    vm->t_str = lk_obj_alloc(vm->t_seq);
    vec_fin(VEC(vm->t_str));
    vec_init(VEC(vm->t_str), sizeof(uint8_t), 16);
}

// new
lk_str_t *lk_str_new(lk_vm_t *vm) {
    return LK_STRING(lk_obj_alloc(vm->t_str));
}

lk_str_t *lk_str_new_from_darray(lk_vm_t *vm, vec_t *list) {
    lk_str_t *self = LK_STRING(lk_obj_alloc(vm->t_str));

    vec_copy(VEC(self), list);
    return self;
}

lk_str_t *lk_str_new_from_data(lk_vm_t *vm, const void *data, int len) {
    vec_t *l = vec_str_alloc_fromdata(data, len);
    lk_str_t *s = lk_str_new_from_darray(vm, l);

    vec_free(l);
    return s;
}

lk_str_t *lk_str_new_from_cstr(lk_vm_t *vm, const char *cstr) {
    return lk_str_new_from_data(vm, cstr, strlen(cstr));
}

// update
void lk_str_set_at_char(lk_str_t *self, lk_num_t *at, lk_char_t *replacement) {
    vec_str_set(VEC(self), CSIZE(at), CHAR(replacement));
}

void lk_str_set_range_str(lk_str_t *self, lk_num_t *from, lk_num_t *to, lk_str_t *replacement) {
    vec_t *x = VEC(self), *y = VEC(replacement);

    vec_resizeitem(x, y);
    vec_setrange(x, CSIZE(from), CSIZE(to), y);
}

void lk_str_unescape(lk_str_t *self) {
    vec_t *data = VEC(self);
    int i;
    uint32_t c;

    for (i = 0; (i = vec_str_find(data, '\\', i)) >= 0; i++) {
        vec_str_remove(data, i);
        switch (c = vec_str_get(data, i)) {
        case 'n':
            vec_str_set(data, i, '\012');
            break;
        case 'r':
            vec_str_set(data, i, '\015');
            break;
        case 't':
            vec_str_set(data, i, '\t');
            break;
        default:
            vec_str_set(data, i, c);
            break;
        }
    }
}

// info
lk_char_t *lk_str_at(lk_str_t *self, lk_num_t *at) {
    return lk_char_new(VM, vec_str_get(VEC(self), CSIZE(at)));
}

lk_obj_t *lk_str_find_char_starting(lk_str_t *self, lk_char_t *pattern, lk_num_t *starting) {
    int i = vec_str_find(VEC(self), CHAR(pattern), CSIZE(starting));
    return i >= 0 ? LK_OBJ(lk_num_new(VM, i)) : NIL;
}

lk_obj_t *lk_str_find_charset_starting(lk_str_t *self, lk_charset_t *pattern, lk_num_t *starting) {
    int i = vec_str_findset(VEC(self), CHARSET(pattern), CSIZE(starting));
    return i >= 0 ? LK_OBJ(lk_num_new(VM, i)) : NIL;
}

lk_obj_t *lk_str_find_str_starting(lk_str_t *self, lk_str_t *pattern, lk_num_t *starting) {
    int i = vec_find_vec(VEC(self), VEC(pattern), CSIZE(starting));
    return i >= 0 ? LK_OBJ(lk_num_new(VM, i)) : NIL;
}

lk_charset_t *lk_str_to_charset(lk_str_t *self) {
    lk_charset_t *charset = lk_charset_new(VM);

    charset_add_darray(CHARSET(charset), VEC(self));
    return charset;
}

lk_num_t *lk_str_to_num(lk_str_t *self) {
    num_t num;

    switch (num_new(VEC(self), &num)) {
    case NUM_TYPE_INT:
        return lk_num_new(VM, num.i);
    case NUM_TYPE_DOUBLE:
        return lk_num_new(VM, num.d);
    default:
        BUG("Invalid num type while trying to parse code.\n");
    }
}

// bind all c funcs to lk equiv
void lk_str_lib_init(lk_vm_t *vm) {
    lk_obj_t *str = vm->t_str, *num = vm->t_num, *charset = vm->t_charset, *ch = vm->t_char;

    lk_global_set("Newline", LK_OBJ(lk_str_new_from_cstr(vm, "\n")));
    lk_global_set("String", str);

    // update
    lk_obj_set_cfunc_cvoid(str, "set!", lk_str_set_at_char, num, ch, NULL);
    lk_obj_set_cfunc_cvoid(str, "set!", lk_str_set_range_str, num, num, str, NULL);

    // info
    lk_obj_set_cfunc_creturn(str, "at", lk_str_at, num, NULL);
    lk_obj_set_cfunc_creturn(str, "find", lk_str_find_char_starting, ch, num, NULL);
    lk_obj_set_cfunc_creturn(str, "find", lk_str_find_charset_starting, charset, num, NULL);
    lk_obj_set_cfunc_creturn(str, "find", lk_str_find_str_starting, str, num, NULL);
    lk_obj_set_cfunc_creturn(str, "toCharacterSet", lk_str_to_charset, NULL);
    lk_obj_set_cfunc_creturn(str, "toNumber", lk_str_to_num, NULL);
}
