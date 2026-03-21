#include "lib.h"

// type
static LK_OBJ_DEFMARKFUNC(mark_list) {
    VEC_EACH_PTR(VEC(self), i, v, mark(v));
}

void lk_list_type_init(lk_vm_t *vm) {
    vm->t_list = lk_obj_alloc(vm->t_seq);
    vec_fin(VEC(vm->t_list));
    vec_init(VEC(vm->t_list), sizeof(lk_obj_t *), 16);
    lk_obj_set_mark_func(vm->t_list, mark_list);
}

// new
lk_list_t *lk_list_new(lk_vm_t *vm) {
    return LK_VEC(lk_obj_alloc(vm->t_list));
}

lk_list_t *lk_list_new_from_darray(lk_vm_t *vm, vec_t *from) {
    lk_list_t *self = lk_list_new(vm);
    vec_copy(VEC(self), from);
    return self;
}

lk_list_t *lk_list_new_from_argv(lk_vm_t *vm, int argc, const char **argv) {
    lk_list_t *self = lk_list_new(vm);
    int i = 0;

    for (; i < argc; i++) {
        vec_ptr_push(VEC(self), lk_str_new_from_cstr(vm, argv[i]));
    }

    return self;
}

// update
void lk_list_insert_num_obj(lk_list_t *self, lk_num_t *index, lk_obj_t *value) {
    vec_ptr_insert(VEC(self), CSIZE(index), lk_obj_add_ref(LK_OBJ(self), value));
}

void lk_list_remove_num(lk_list_t *self, lk_num_t *index) {
    vec_ptr_remove(VEC(self), CSIZE(index));
}

void lk_list_set_num_obj(lk_list_t *self, lk_num_t *index, lk_obj_t *value) {
    vec_ptr_set(VEC(self), CSIZE(index), value);
}

void lk_list_set_num_num_list(lk_list_t *self, lk_num_t *from, lk_num_t *to, lk_list_t *list) {
    vec_setrange(VEC(self), CSIZE(from), CSIZE(to), VEC(list));
}

// info
lk_obj_t *lk_list_at_num(lk_list_t *self, lk_num_t *index) {
    lk_obj_t *value = vec_ptr_get(VEC(self), CSIZE(index));
    return value != NULL ? value : NIL;
}

void lk_list_flatten(lk_obj_t *self, lk_scope_t *local) {
    lk_scope_t *caller = local->caller;
    if (!VEC_ISINIT(&caller->stack))
        vec_ptr_init(&caller->stack);
    vec_concat(&caller->stack, VEC(self));
}

// bind all c funcs to lk equiv
void lk_list_lib_init(lk_vm_t *vm) {
    lk_obj_t *list = vm->t_list, *obj = vm->t_obj, *num = vm->t_num;
    lk_global_set("List", list);

    // update
    lk_obj_set_cfunc_cvoid(list, "insert!", lk_list_insert_num_obj, num, obj, NULL);
    lk_obj_set_cfunc_cvoid(list, "remove!", lk_list_remove_num, num, NULL);
    lk_obj_set_cfunc_cvoid(list, "set!", lk_list_set_num_obj, num, obj, NULL);
    lk_obj_set_cfunc_cvoid(list, "set!", lk_list_set_num_num_list, num, num, list, NULL);

    // info
    lk_obj_set_cfunc_creturn(list, "at", lk_list_at_num, num, NULL);
    lk_obj_set_cfunc_lk(list, "*", lk_list_flatten, NULL);
}
