#include "seq.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__seq) {
    darray_copy(LIST(self), LIST(parent));
}
static LK_OBJ_DEFFREEFUNC(free__seq) {
    darray_fin(LIST(self));
}
LK_EXT_DEFINIT(lk_seq_extinittypes) {
    vm->t_seq = lk_object_allocwithsize(vm->t_obj, sizeof(lk_seq_t));
    darray_init(LIST(vm->t_seq), 1, 16);
    lk_object_setallocfunc(vm->t_seq, alloc__seq);
    lk_object_setfreefunc(vm->t_seq, free__seq);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(at__gl_vec) {
    lk_list_t *ret = LK_LIST(lk_object_clone(self));
    darray_t *sl = LIST(self), *rl = LIST(ret), *indexes = LIST(ARG(0));
    darray_limit(rl, LIST_COUNT(indexes));
    LIST_EACH(indexes, i, v,
        darray_set(rl, i, darray_get(sl, *(int *)v));
    );
    RETURN(ret);
}
LK_LIBRARY_DEFINECFUNCTION(clearB__gl) {
    darray_clear(LIST(self)); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(cmp__gl_gl) {
    RETURN(lk_fi_new(VM, darray_compareTo(LIST(self), LIST(ARG(0))))); }
LK_LIBRARY_DEFINECFUNCTION(concatB__gl_gl) {
    darray_concat(LIST(self), LIST(ARG(0))); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(size__gl) {
    RETURN(lk_fi_new(VM, LIST_COUNT(LIST(self)))); }
LK_LIBRARY_DEFINECFUNCTION(eq__gl_gl) {
    RETURN(LIST_EQ(LIST(self), LIST(ARG(0))) ? T : F); }
LK_LIBRARY_DEFINECFUNCTION(limitB__gl_fi) {
    darray_limit(LIST(self), INT(ARG(0))); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(offsetB__gl_fi) {
    darray_offset(LIST(self), INT(ARG(0))); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(restB__gl) {
    darray_offset(LIST(self), 1); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(reverseB__gl) {
    darray_reverse(LIST(self)); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(sliceB__gl_fi_fi) {
    darray_slice(LIST(self), INT(ARG(0)), INT(ARG(1))); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(swapB__gl_fi_fi) {
    int s = LIST(self)->data->ilen;
    void *x = darray_get(LIST(self), INT(ARG(0)));
    void *y = darray_get(LIST(self), INT(ARG(1)));
    void *t = memory_alloc(s);
    memcpy(t, x, s);
    memcpy(x, y, s);
    memcpy(y, t, s);
    memory_free(t);
    RETURN(self);
}
LK_EXT_DEFINIT(lk_seq_extinitfuncs) {
    lk_object_t *gl = vm->t_seq, *fi = vm->t_fi, *vec = vm->t_vector;
    lk_library_setGlobal("Sequence", gl);
    lk_library_setCFunction(gl, "at", at__gl_vec, vec, NULL);
    lk_library_setCFunction(gl, "clear!", clearB__gl, NULL);
    lk_library_setCFunction(gl, "<=>", cmp__gl_gl, gl, NULL);
    lk_library_setCFunction(gl, "++=", concatB__gl_gl, gl, NULL);
    lk_library_setCFunction(gl, "size", size__gl, NULL);
    lk_library_setCFunction(gl, "==", eq__gl_gl, gl, NULL);
    lk_library_setCFunction(gl, "limit!", limitB__gl_fi, fi, NULL);
    lk_library_setCFunction(gl, "offset!", offsetB__gl_fi, fi, NULL);
    lk_library_setCFunction(gl, "rest!", restB__gl, NULL);
    lk_library_setCFunction(gl, "reverse!", reverseB__gl, NULL);
    lk_library_setCFunction(gl, "slice!", sliceB__gl_fi_fi, fi, fi, NULL);
    lk_library_setCFunction(gl, "swap!", swapB__gl_fi_fi, fi, fi, NULL);
}
