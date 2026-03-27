#include "seq.h"
#include "lib.h"
#include "num.h"

// ext map - types
static void alloc_seq(lk_obj_t *self, lk_obj_t *parent) {
    vec_copy(VEC(self), VEC(parent));
}

static void free_seq(lk_obj_t *self) {
    vec_fin(VEC(self));
}

void lk_seq_type_init(lk_vm_t *vm) {
    vm->t_seq = lk_obj_alloc_type(vm->t_obj, sizeof(lk_seq_t));
    vec_init(VEC(vm->t_seq), 1, 16);
    lk_obj_set_alloc_func(vm->t_seq, alloc_seq);
    lk_obj_set_free_func(vm->t_seq, free_seq);
}

// ext map - funcs
static void at_gl_vec(lk_obj_t *self, lk_scope_t *local) {
    lk_list_t *ret = LK_VEC(lk_obj_clone(self));
    vec_t *sl = VEC(self), *rl = VEC(ret), *indexes = VEC(ARG(0));

    vec_limit(rl, VEC_COUNT(indexes));
    VEC_EACH(indexes, i, v, vec_set(rl, i, vec_get(sl, *(int *)v)););
    RETURN(ret);
}

static void clearB_gl(lk_obj_t *self, lk_scope_t *local) {
    vec_clear(VEC(self));
    RETURN(self);
}

static void cmp_gl_gl(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, vec_cmp(VEC(self), VEC(ARG(0)))));
}

static void concatB_gl_gl(lk_obj_t *self, lk_scope_t *local) {
    vec_concat(VEC(self), VEC(ARG(0)));
    RETURN(self);
}

static void size_gl(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, VEC_COUNT(VEC(self))));
}

static void eq_gl_gl(lk_obj_t *self, lk_scope_t *local) {
    RETURN(VEC_EQ(VEC(self), VEC(ARG(0))) ? TRUE : FALSE);
}

static void limitB_gl_num(lk_obj_t *self, lk_scope_t *local) {
    vec_limit(VEC(self), CSIZE(ARG(0)));
    RETURN(self);
}

static void offsetB_gl_num(lk_obj_t *self, lk_scope_t *local) {
    vec_offset(VEC(self), CSIZE(ARG(0)));
    RETURN(self);
}

static void restB_gl(lk_obj_t *self, lk_scope_t *local) {
    vec_offset(VEC(self), 1);
    RETURN(self);
}

static void reverseB_gl(lk_obj_t *self, lk_scope_t *local) {
    vec_reverse(VEC(self));
    RETURN(self);
}

static void sliceB_gl_num_num(lk_obj_t *self, lk_scope_t *local) {
    vec_slice(VEC(self), CSIZE(ARG(0)), CSIZE(ARG(1)));
    RETURN(self);
}

static void swapB_gl_num_num(lk_obj_t *self, lk_scope_t *local) {
    int s = VEC(self)->buf->item_size;
    void *x = vec_get(VEC(self), CSIZE(ARG(0)));
    void *y = vec_get(VEC(self), CSIZE(ARG(1)));
    void *t = mem_alloc(s);

    memcpy(t, x, s);
    memcpy(x, y, s);
    memcpy(y, t, s);
    mem_free(t);
    RETURN(self);
}

void lk_seq_lib_init(lk_vm_t *vm) {
    lk_obj_t *gl = vm->t_seq, *num = vm->t_num, *vec = vm->t_vec;

    lk_global_set("Sequence", gl);
    lk_obj_set_cfunc_lk(gl, "at", at_gl_vec, vec, NULL);
    lk_obj_set_cfunc_lk(gl, "clear!", clearB_gl, NULL);
    lk_obj_set_cfunc_lk(gl, "<=>", cmp_gl_gl, gl, NULL);
    lk_obj_set_cfunc_lk(gl, "++=", concatB_gl_gl, gl, NULL);
    lk_obj_set_cfunc_lk(gl, "size", size_gl, NULL);
    lk_obj_set_cfunc_lk(gl, "==", eq_gl_gl, gl, NULL);
    lk_obj_set_cfunc_lk(gl, "limit!", limitB_gl_num, num, NULL);
    lk_obj_set_cfunc_lk(gl, "offset!", offsetB_gl_num, num, NULL);
    lk_obj_set_cfunc_lk(gl, "rest!", restB_gl, NULL);
    lk_obj_set_cfunc_lk(gl, "reverse!", reverseB_gl, NULL);
    lk_obj_set_cfunc_lk(gl, "slice!", sliceB_gl_num_num, num, num, NULL);
    lk_obj_set_cfunc_lk(gl, "swap!", swapB_gl_num_num, num, num, NULL);
}
