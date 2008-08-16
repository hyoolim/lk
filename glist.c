#include "glist.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__glist) {
    Sequence_copy(LIST(self), LIST(parent));
}
static LK_OBJ_DEFFREEFUNC(free__glist) {
    Sequence_fin(LIST(self));
}
LK_EXT_DEFINIT(lk_GSequence_extinittypes) {
    vm->t_glist = lk_Object_allocwithsize(vm->t_obj, sizeof(lk_GSequence_t));
    Sequence_init(LIST(vm->t_glist), 1, 16);
    lk_Object_setallocfunc(vm->t_glist, alloc__glist);
    lk_Object_setfreefunc(vm->t_glist, free__glist);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(at__gl_vec) {
    lk_List_t *ret = LK_LIST(lk_Object_clone(self));
    Sequence_t *sl = LIST(self), *rl = LIST(ret), *indexes = LIST(ARG(0));
    Sequence_limit(rl, LIST_COUNT(indexes));
    LIST_EACH(indexes, i, v,
        Sequence_set(rl, i, Sequence_get(sl, *(int *)v));
    );
    RETURN(ret);
}
LK_LIBRARY_DEFINECFUNCTION(chopB__gl) {
    Sequence_limit(LIST(self), -1); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(clearB__gl) {
    Sequence_clear(LIST(self)); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(cmp__gl_gl) {
    RETURN(lk_Fi_new(VM, Sequence_cmp(LIST(self), LIST(ARG(0))))); }
LK_LIBRARY_DEFINECFUNCTION(concatB__gl_gl) {
    Sequence_concat(LIST(self), LIST(ARG(0))); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(count__gl) {
    RETURN(lk_Fi_new(VM, LIST_COUNT(LIST(self)))); }
LK_LIBRARY_DEFINECFUNCTION(eq__gl_gl) {
    RETURN(LIST_EQ(LIST(self), LIST(ARG(0))) ? T : F); }
LK_LIBRARY_DEFINECFUNCTION(limitB__gl_fi) {
    Sequence_limit(LIST(self), INT(ARG(0))); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(offsetB__gl_fi) {
    Sequence_offset(LIST(self), INT(ARG(0))); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(restB__gl) {
    Sequence_offset(LIST(self), 1); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(reverseB__gl) {
    Sequence_reverse(LIST(self)); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(sliceB__gl_fi_fi) {
    Sequence_slice(LIST(self), INT(ARG(0)), INT(ARG(1))); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(swapB__gl_fi_fi) {
    int s = LIST(self)->data->ilen;
    void *x = Sequence_get(LIST(self), INT(ARG(0)));
    void *y = Sequence_get(LIST(self), INT(ARG(1)));
    void *t = memory_alloc(s);
    memcpy(t, x, s);
    memcpy(x, y, s);
    memcpy(y, t, s);
    memory_free(t);
    RETURN(self);
}
LK_EXT_DEFINIT(lk_GSequence_extinitfuncs) {
    lk_Object_t *gl = vm->t_glist, *fi = vm->t_fi, *vec = vm->t_vector;
    lk_Library_setGlobal("GenericList", gl);
    lk_Library_setCFunction(gl, "at", at__gl_vec, vec, NULL);
    lk_Library_setCFunction(gl, "chop!", chopB__gl, NULL);
    lk_Library_setCFunction(gl, "clear!", clearB__gl, NULL);
    lk_Library_setCFunction(gl, "<=>", cmp__gl_gl, gl, NULL);
    lk_Library_setCFunction(gl, "++=", concatB__gl_gl, gl, NULL);
    lk_Library_setCFunction(gl, "count", count__gl, NULL);
    lk_Library_setCFunction(gl, "==", eq__gl_gl, gl, NULL);
    lk_Library_setCFunction(gl, "limit!", limitB__gl_fi, fi, NULL);
    lk_Library_setCFunction(gl, "offset!", offsetB__gl_fi, fi, NULL);
    lk_Library_setCFunction(gl, "rest!", restB__gl, NULL);
    lk_Library_setCFunction(gl, "reverse!", reverseB__gl, NULL);
    lk_Library_setCFunction(gl, "slice!", sliceB__gl_fi_fi, fi, fi, NULL);
    lk_Library_setCFunction(gl, "swap!", swapB__gl_fi_fi, fi, fi, NULL);
}
