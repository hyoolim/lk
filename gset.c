#include "ext.h"
#include "fixnum.h"
#include "gset.h"
#include "list.h"

/* ext map - types */
static LK_OBJECT_DEFALLOCFUNC(alloc__gset) {
    pt_set_copy(SET(self), SET(proto));
}
static LK_OBJECT_DEFMARKFUNC(mark__gset) {
    PT_SET_EACH(SET(self), i, mark(LK_O(i->key)));
}
static LK_OBJECT_DEFFREEFUNC(free__gset) {
    pt_set_fin(SET(self));
}
LK_EXT_DEFINIT(lk_gset_extinittypes) {
    vm->t_gset = lk_object_allocwithsize(vm->t_object, sizeof(lk_gset_t));
    pt_set_init(SET(vm->t_gset), 0, lk_object_hashcode, lk_object_keycmp);
    lk_object_setallocfunc(vm->t_gset, alloc__gset);
    lk_object_setmarkfunc(vm->t_gset, mark__gset);
    lk_object_setfreefunc(vm->t_gset, free__gset);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(clearB__gset) {
    pt_set_clear(SET(self));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(count__gset) {
    RETURN(lk_fi_new(VM, pt_set_count(SET(self))));
}
static LK_EXT_DEFCFUNC(keys__gset) {
    lk_list_t *keys = lk_list_new(VM);
    PT_SET_EACH(SET(self), i, pt_list_pushptr(LIST(keys), (void *)i->key));
    RETURN(keys);
}
LK_EXT_DEFINIT(lk_gset_extinitfuncs) {
    lk_object_t *gset = vm->t_gset;
    lk_ext_global("GenericSet", gset);
    lk_ext_cfunc(gset, "clear!", clearB__gset, NULL);
    lk_ext_cfunc(gset, "count", count__gset, NULL);
    lk_ext_cfunc(gset, "keys", keys__gset, NULL);
}
