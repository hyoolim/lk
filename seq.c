#include "seq.h"
#include "ext.h"
#include "number.h"

/* ext map - types */
static void alloc_seq(lk_object_t *self, lk_object_t *parent) {
    darray_copy(DARRAY(self), DARRAY(parent));
}
static void free_seq(lk_object_t *self) {
    darray_fin(DARRAY(self));
}
void lk_seq_typeinit(lk_vm_t *vm) {
    vm->t_seq = lk_object_allocWithSize(vm->t_object, sizeof(lk_seq_t));
    darray_init(DARRAY(vm->t_seq), 1, 16);
    lk_object_setallocfunc(vm->t_seq, alloc_seq);
    lk_object_setfreefunc(vm->t_seq, free_seq);
}

/* ext map - funcs */
static void at_gl_vec(lk_object_t *self, lk_scope_t *local) {
    lk_list_t *ret = LK_DARRAY(lk_object_clone(self));
    darray_t *sl = DARRAY(self), *rl = DARRAY(ret), *indexes = DARRAY(ARG(0));
    darray_limit(rl, LIST_COUNT(indexes));
    LIST_EACH(indexes, i, v,
        darray_set(rl, i, darray_get(sl, *(int *)v));
    );
    RETURN(ret);
}
static void clearB_gl(lk_object_t *self, lk_scope_t *local) {
    darray_clear(DARRAY(self)); RETURN(self); }
static void cmp_gl_gl(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_number_new(VM, darray_compareTo(DARRAY(self), DARRAY(ARG(0))))); }
static void concatB_gl_gl(lk_object_t *self, lk_scope_t *local) {
    darray_concat(DARRAY(self), DARRAY(ARG(0))); RETURN(self); }
static void size_gl(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_number_new(VM, LIST_COUNT(DARRAY(self)))); }
static void eq_gl_gl(lk_object_t *self, lk_scope_t *local) {
    RETURN(LIST_EQ(DARRAY(self), DARRAY(ARG(0))) ? TRUE : FALSE); }
static void limitB_gl_number(lk_object_t *self, lk_scope_t *local) {
    darray_limit(DARRAY(self), CSIZE(ARG(0))); RETURN(self); }
static void offsetB_gl_number(lk_object_t *self, lk_scope_t *local) {
    darray_offset(DARRAY(self), CSIZE(ARG(0))); RETURN(self); }
static void restB_gl(lk_object_t *self, lk_scope_t *local) {
    darray_offset(DARRAY(self), 1); RETURN(self); }
static void reverseB_gl(lk_object_t *self, lk_scope_t *local) {
    darray_reverse(DARRAY(self)); RETURN(self); }
static void sliceB_gl_number_number(lk_object_t *self, lk_scope_t *local) {
    darray_slice(DARRAY(self), CSIZE(ARG(0)), CSIZE(ARG(1))); RETURN(self); }
static void swapB_gl_number_number(lk_object_t *self, lk_scope_t *local) {
    int s = DARRAY(self)->data->ilen;
    void *x = darray_get(DARRAY(self), CSIZE(ARG(0)));
    void *y = darray_get(DARRAY(self), CSIZE(ARG(1)));
    void *t = memory_alloc(s);
    memcpy(t, x, s);
    memcpy(x, y, s);
    memcpy(y, t, s);
    memory_free(t);
    RETURN(self);
}
void lk_seq_libinit(lk_vm_t *vm) {
    lk_object_t *gl = vm->t_seq, *number = vm->t_number, *vec = vm->t_vector;
    lk_lib_setGlobal("Sequence", gl);
    lk_object_set_cfunc_lk(gl, "at", at_gl_vec, vec, NULL);
    lk_object_set_cfunc_lk(gl, "clear!", clearB_gl, NULL);
    lk_object_set_cfunc_lk(gl, "<=>", cmp_gl_gl, gl, NULL);
    lk_object_set_cfunc_lk(gl, "++=", concatB_gl_gl, gl, NULL);
    lk_object_set_cfunc_lk(gl, "size", size_gl, NULL);
    lk_object_set_cfunc_lk(gl, "==", eq_gl_gl, gl, NULL);
    lk_object_set_cfunc_lk(gl, "limit!", limitB_gl_number, number, NULL);
    lk_object_set_cfunc_lk(gl, "offset!", offsetB_gl_number, number, NULL);
    lk_object_set_cfunc_lk(gl, "rest!", restB_gl, NULL);
    lk_object_set_cfunc_lk(gl, "reverse!", reverseB_gl, NULL);
    lk_object_set_cfunc_lk(gl, "slice!", sliceB_gl_number_number, number, number, NULL);
    lk_object_set_cfunc_lk(gl, "swap!", swapB_gl_number_number, number, number, NULL);
}
