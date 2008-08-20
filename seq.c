#include "seq.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__seq) {
    darray_copy(DARRAY(self), DARRAY(parent));
}
static LK_OBJ_DEFFREEFUNC(free__seq) {
    darray_fin(DARRAY(self));
}
LK_LIB_DEFINEINIT(lk_seq_libPreInit) {
    vm->t_seq = lk_object_allocwithsize(vm->t_obj, sizeof(lk_seq_t));
    darray_init(DARRAY(vm->t_seq), 1, 16);
    lk_object_setallocfunc(vm->t_seq, alloc__seq);
    lk_object_setfreefunc(vm->t_seq, free__seq);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(at__gl_vec) {
    lk_list_t *ret = LK_DARRAY(lk_object_clone(self));
    darray_t *sl = DARRAY(self), *rl = DARRAY(ret), *indexes = DARRAY(ARG(0));
    darray_limit(rl, LIST_COUNT(indexes));
    LIST_EACH(indexes, i, v,
        darray_set(rl, i, darray_get(sl, *(int *)v));
    );
    RETURN(ret);
}
LK_LIB_DEFINECFUNC(clearB__gl) {
    darray_clear(DARRAY(self)); RETURN(self); }
LK_LIB_DEFINECFUNC(cmp__gl_gl) {
    RETURN(lk_fi_new(VM, darray_compareTo(DARRAY(self), DARRAY(ARG(0))))); }
LK_LIB_DEFINECFUNC(concatB__gl_gl) {
    darray_concat(DARRAY(self), DARRAY(ARG(0))); RETURN(self); }
LK_LIB_DEFINECFUNC(size__gl) {
    RETURN(lk_fi_new(VM, LIST_COUNT(DARRAY(self)))); }
LK_LIB_DEFINECFUNC(eq__gl_gl) {
    RETURN(LIST_EQ(DARRAY(self), DARRAY(ARG(0))) ? TRUE : FALSE); }
LK_LIB_DEFINECFUNC(limitB__gl_fi) {
    darray_limit(DARRAY(self), INT(ARG(0))); RETURN(self); }
LK_LIB_DEFINECFUNC(offsetB__gl_fi) {
    darray_offset(DARRAY(self), INT(ARG(0))); RETURN(self); }
LK_LIB_DEFINECFUNC(restB__gl) {
    darray_offset(DARRAY(self), 1); RETURN(self); }
LK_LIB_DEFINECFUNC(reverseB__gl) {
    darray_reverse(DARRAY(self)); RETURN(self); }
LK_LIB_DEFINECFUNC(sliceB__gl_fi_fi) {
    darray_slice(DARRAY(self), INT(ARG(0)), INT(ARG(1))); RETURN(self); }
LK_LIB_DEFINECFUNC(swapB__gl_fi_fi) {
    int s = DARRAY(self)->data->ilen;
    void *x = darray_get(DARRAY(self), INT(ARG(0)));
    void *y = darray_get(DARRAY(self), INT(ARG(1)));
    void *t = memory_alloc(s);
    memcpy(t, x, s);
    memcpy(x, y, s);
    memcpy(y, t, s);
    memory_free(t);
    RETURN(self);
}
LK_LIB_DEFINEINIT(lk_seq_libInit) {
    lk_object_t *gl = vm->t_seq, *fi = vm->t_fi, *vec = vm->t_vector;
    lk_lib_setGlobal("Sequence", gl);
    lk_lib_setCFunc(gl, "at", at__gl_vec, vec, NULL);
    lk_lib_setCFunc(gl, "clear!", clearB__gl, NULL);
    lk_lib_setCFunc(gl, "<=>", cmp__gl_gl, gl, NULL);
    lk_lib_setCFunc(gl, "++=", concatB__gl_gl, gl, NULL);
    lk_lib_setCFunc(gl, "size", size__gl, NULL);
    lk_lib_setCFunc(gl, "==", eq__gl_gl, gl, NULL);
    lk_lib_setCFunc(gl, "limit!", limitB__gl_fi, fi, NULL);
    lk_lib_setCFunc(gl, "offset!", offsetB__gl_fi, fi, NULL);
    lk_lib_setCFunc(gl, "rest!", restB__gl, NULL);
    lk_lib_setCFunc(gl, "reverse!", reverseB__gl, NULL);
    lk_lib_setCFunc(gl, "slice!", sliceB__gl_fi_fi, fi, fi, NULL);
    lk_lib_setCFunc(gl, "swap!", swapB__gl_fi_fi, fi, fi, NULL);
}
