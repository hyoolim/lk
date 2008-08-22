#include "seq.h"
#include "ext.h"
#include "number.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc_seq) {
    darray_copy(DARRAY(self), DARRAY(parent));
}
static LK_OBJ_DEFFREEFUNC(free_seq) {
    darray_fin(DARRAY(self));
}
LK_LIB_DEFINEINIT(lk_seq_libPreInit) {
    vm->t_seq = lk_object_allocWithSize(vm->t_object, sizeof(lk_seq_t));
    darray_init(DARRAY(vm->t_seq), 1, 16);
    lk_object_setallocfunc(vm->t_seq, alloc_seq);
    lk_object_setfreefunc(vm->t_seq, free_seq);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(at_gl_vec) {
    lk_list_t *ret = LK_DARRAY(lk_object_clone(self));
    darray_t *sl = DARRAY(self), *rl = DARRAY(ret), *indexes = DARRAY(ARG(0));
    darray_limit(rl, LIST_COUNT(indexes));
    LIST_EACH(indexes, i, v,
        darray_set(rl, i, darray_get(sl, *(int *)v));
    );
    RETURN(ret);
}
LK_LIB_DEFINECFUNC(clearB_gl) {
    darray_clear(DARRAY(self)); RETURN(self); }
LK_LIB_DEFINECFUNC(cmp_gl_gl) {
    RETURN(lk_number_new(VM, darray_compareTo(DARRAY(self), DARRAY(ARG(0))))); }
LK_LIB_DEFINECFUNC(concatB_gl_gl) {
    darray_concat(DARRAY(self), DARRAY(ARG(0))); RETURN(self); }
LK_LIB_DEFINECFUNC(size_gl) {
    RETURN(lk_number_new(VM, LIST_COUNT(DARRAY(self)))); }
LK_LIB_DEFINECFUNC(eq_gl_gl) {
    RETURN(LIST_EQ(DARRAY(self), DARRAY(ARG(0))) ? TRUE : FALSE); }
LK_LIB_DEFINECFUNC(limitB_gl_number) {
    darray_limit(DARRAY(self), CSIZE(ARG(0))); RETURN(self); }
LK_LIB_DEFINECFUNC(offsetB_gl_number) {
    darray_offset(DARRAY(self), CSIZE(ARG(0))); RETURN(self); }
LK_LIB_DEFINECFUNC(restB_gl) {
    darray_offset(DARRAY(self), 1); RETURN(self); }
LK_LIB_DEFINECFUNC(reverseB_gl) {
    darray_reverse(DARRAY(self)); RETURN(self); }
LK_LIB_DEFINECFUNC(sliceB_gl_number_number) {
    darray_slice(DARRAY(self), CSIZE(ARG(0)), CSIZE(ARG(1))); RETURN(self); }
LK_LIB_DEFINECFUNC(swapB_gl_number_number) {
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
LK_LIB_DEFINEINIT(lk_seq_libInit) {
    lk_object_t *gl = vm->t_seq, *number = vm->t_number, *vec = vm->t_vector;
    lk_lib_setGlobal("Sequence", gl);
    lk_lib_setCFunc(gl, "at", at_gl_vec, vec, NULL);
    lk_lib_setCFunc(gl, "clear!", clearB_gl, NULL);
    lk_lib_setCFunc(gl, "<=>", cmp_gl_gl, gl, NULL);
    lk_lib_setCFunc(gl, "++=", concatB_gl_gl, gl, NULL);
    lk_lib_setCFunc(gl, "size", size_gl, NULL);
    lk_lib_setCFunc(gl, "==", eq_gl_gl, gl, NULL);
    lk_lib_setCFunc(gl, "limit!", limitB_gl_number, number, NULL);
    lk_lib_setCFunc(gl, "offset!", offsetB_gl_number, number, NULL);
    lk_lib_setCFunc(gl, "rest!", restB_gl, NULL);
    lk_lib_setCFunc(gl, "reverse!", reverseB_gl, NULL);
    lk_lib_setCFunc(gl, "slice!", sliceB_gl_number_number, number, number, NULL);
    lk_lib_setCFunc(gl, "swap!", swapB_gl_number_number, number, number, NULL);
}
