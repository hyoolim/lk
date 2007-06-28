#include "glist.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__glist) {
    list_copy(LIST(self), LIST(proto));
}
static LK_OBJ_DEFFREEFUNC(free__glist) {
    list_fin(LIST(self));
}
LK_EXT_DEFINIT(lk_glist_extinittypes) {
    vm->t_glist = lk_obj_allocwithsize(vm->t_obj, sizeof(lk_glist_t));
    list_init(LIST(vm->t_glist), 1, 16);
    lk_obj_setallocfunc(vm->t_glist, alloc__glist);
    lk_obj_setfreefunc(vm->t_glist, free__glist);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(at__gl_vec) {
    lk_list_t *ret = LK_LIST(lk_obj_clone(self));
    list_t *sl = LIST(self), *rl = LIST(ret), *indexes = LIST(ARG(0));
    list_limit(rl, LIST_COUNT(indexes));
    LIST_EACH(indexes, i, v,
        list_set(rl, i, list_get(sl, *(int *)v));
    );
    RETURN(ret);
}
static LK_EXT_DEFCFUNC(chopB__gl) {
    list_limit(LIST(self), -1); RETURN(self); }
static LK_EXT_DEFCFUNC(clearB__gl) {
    list_clear(LIST(self)); RETURN(self); }
static LK_EXT_DEFCFUNC(cmp__gl_gl) {
    RETURN(lk_fi_new(VM, list_cmp(LIST(self), LIST(ARG(0))))); }
static LK_EXT_DEFCFUNC(concatB__gl_gl) {
    list_concat(LIST(self), LIST(ARG(0))); RETURN(self); }
static LK_EXT_DEFCFUNC(count__gl) {
    RETURN(lk_fi_new(VM, LIST_COUNT(LIST(self)))); }
static LK_EXT_DEFCFUNC(eq__gl_gl) {
    RETURN(LIST_EQ(LIST(self), LIST(ARG(0))) ? T : F); }
static LK_EXT_DEFCFUNC(limitB__gl_fi) {
    list_limit(LIST(self), INT(ARG(0))); RETURN(self); }
static LK_EXT_DEFCFUNC(offsetB__gl_fi) {
    list_offset(LIST(self), INT(ARG(0))); RETURN(self); }
static LK_EXT_DEFCFUNC(restB__gl) {
    list_offset(LIST(self), 1); RETURN(self); }
static LK_EXT_DEFCFUNC(reverseB__gl) {
    list_reverse(LIST(self)); RETURN(self); }
static LK_EXT_DEFCFUNC(sliceB__gl_fi_fi) {
    list_slice(LIST(self), INT(ARG(0)), INT(ARG(1))); RETURN(self); }
static LK_EXT_DEFCFUNC(swapB__gl_fi_fi) {
    int s = LIST(self)->data->ilen;
    void *x = list_get(LIST(self), INT(ARG(0)));
    void *y = list_get(LIST(self), INT(ARG(1)));
    void *t = memory_alloc(s);
    memcpy(t, x, s);
    memcpy(x, y, s);
    memcpy(y, t, s);
    memory_free(t);
    RETURN(self);
}
LK_EXT_DEFINIT(lk_glist_extinitfuncs) {
    lk_obj_t *gl = vm->t_glist, *fi = vm->t_fi, *vec = vm->t_vector;
    lk_ext_global("GenericList", gl);
    lk_ext_cfunc(gl, "at", at__gl_vec, vec, NULL);
    lk_ext_cfunc(gl, "chop!", chopB__gl, NULL);
    lk_ext_cfunc(gl, "clear!", clearB__gl, NULL);
    lk_ext_cfunc(gl, "cmp", cmp__gl_gl, gl, NULL);
    lk_ext_cfunc(gl, "concat!", concatB__gl_gl, gl, NULL);
    lk_ext_cfunc(gl, "count", count__gl, NULL);
    lk_ext_cfunc(gl, "eq?", eq__gl_gl, gl, NULL);
    lk_ext_cfunc(gl, "limit!", limitB__gl_fi, fi, NULL);
    lk_ext_cfunc(gl, "offset!", offsetB__gl_fi, fi, NULL);
    lk_ext_cfunc(gl, "rest!", restB__gl, NULL);
    lk_ext_cfunc(gl, "reverse!", reverseB__gl, NULL);
    lk_ext_cfunc(gl, "slice!", sliceB__gl_fi_fi, fi, fi, NULL);
    lk_ext_cfunc(gl, "swap!", swapB__gl_fi_fi, fi, fi, NULL);
}
