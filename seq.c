#include "seq.h"
#include "ext.h"
#include "num.h"

/* ext map - types */
static void alloc_seq(lk_obj_t *self, lk_obj_t *parent) {
    darray_copy(DARRAY(self), DARRAY(parent));
}
static void free_seq(lk_obj_t *self) {
    darray_fin(DARRAY(self));
}
void lk_seq_typeinit(lk_vm_t *vm) {
    vm->t_seq = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_seq_t));
    darray_init(DARRAY(vm->t_seq), 1, 16);
    lk_obj_setallocfunc(vm->t_seq, alloc_seq);
    lk_obj_setfreefunc(vm->t_seq, free_seq);
}

/* ext map - funcs */
static void at_gl_vec(lk_obj_t *self, lk_scope_t *local) {
    lk_list_t *ret = LK_DARRAY(lk_obj_clone(self));
    darray_t *sl = DARRAY(self), *rl = DARRAY(ret), *indexes = DARRAY(ARG(0));
    darray_limit(rl, LIST_COUNT(indexes));
    LIST_EACH(indexes, i, v,
        darray_set(rl, i, darray_get(sl, *(int *)v));
    );
    RETURN(ret);
}
static void clearB_gl(lk_obj_t *self, lk_scope_t *local) {
    darray_clear(DARRAY(self)); RETURN(self); }
static void cmp_gl_gl(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, darray_compareTo(DARRAY(self), DARRAY(ARG(0))))); }
static void concatB_gl_gl(lk_obj_t *self, lk_scope_t *local) {
    darray_concat(DARRAY(self), DARRAY(ARG(0))); RETURN(self); }
static void size_gl(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, LIST_COUNT(DARRAY(self)))); }
static void eq_gl_gl(lk_obj_t *self, lk_scope_t *local) {
    RETURN(LIST_EQ(DARRAY(self), DARRAY(ARG(0))) ? TRUE : FALSE); }
static void limitB_gl_num(lk_obj_t *self, lk_scope_t *local) {
    darray_limit(DARRAY(self), CSIZE(ARG(0))); RETURN(self); }
static void offsetB_gl_num(lk_obj_t *self, lk_scope_t *local) {
    darray_offset(DARRAY(self), CSIZE(ARG(0))); RETURN(self); }
static void restB_gl(lk_obj_t *self, lk_scope_t *local) {
    darray_offset(DARRAY(self), 1); RETURN(self); }
static void reverseB_gl(lk_obj_t *self, lk_scope_t *local) {
    darray_reverse(DARRAY(self)); RETURN(self); }
static void sliceB_gl_num_num(lk_obj_t *self, lk_scope_t *local) {
    darray_slice(DARRAY(self), CSIZE(ARG(0)), CSIZE(ARG(1))); RETURN(self); }
static void swapB_gl_num_num(lk_obj_t *self, lk_scope_t *local) {
    int s = DARRAY(self)->data->ilen;
    void *x = darray_get(DARRAY(self), CSIZE(ARG(0)));
    void *y = darray_get(DARRAY(self), CSIZE(ARG(1)));
    void *t = mem_alloc(s);
    memcpy(t, x, s);
    memcpy(x, y, s);
    memcpy(y, t, s);
    mem_free(t);
    RETURN(self);
}
void lk_seq_libinit(lk_vm_t *vm) {
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
