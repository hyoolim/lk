#include "ext.h"
#include "fixnum.h"
#include "gset.h"
#include "list.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__gset) {
    set_copy(SET(self), SET(proto));
}
static LK_OBJ_DEFMARKFUNC(mark__gset) {
    SET_EACH(SET(self), i, mark(LK_OBJ(i->key)));
}
static LK_OBJ_DEFFREEFUNC(free__gset) {
    set_fin(SET(self));
}
LK_EXT_DEFINIT(lk_gset_extinittypes) {
    vm->t_gset = lk_obj_allocwithsize(vm->t_obj, sizeof(lk_gset_t));
    set_init(SET(vm->t_gset), 0, lk_obj_hashcode, lk_obj_keycmp);
    lk_obj_setallocfunc(vm->t_gset, alloc__gset);
    lk_obj_setmarkfunc(vm->t_gset, mark__gset);
    lk_obj_setfreefunc(vm->t_gset, free__gset);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(clearB__gset) {
    set_clear(SET(self));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(count__gset) {
    RETURN(lk_fi_new(VM, set_count(SET(self))));
}
static LK_EXT_DEFCFUNC(keys__gset) {
    lk_list_t *keys = lk_list_new(VM);
    SET_EACH(SET(self), i, list_pushptr(LIST(keys), (void *)i->key));
    RETURN(keys);
}
LK_EXT_DEFINIT(lk_gset_extinitfuncs) {
    lk_obj_t *gset = vm->t_gset;
    lk_ext_global("GenericSet", gset);
    lk_ext_cfunc(gset, "clear!", clearB__gset, NULL);
    lk_ext_cfunc(gset, "count", count__gset, NULL);
    lk_ext_cfunc(gset, "keys", keys__gset, NULL);
}
