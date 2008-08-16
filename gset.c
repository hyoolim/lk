#include "ext.h"
#include "fixnum.h"
#include "gset.h"
#include "list.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__gset) {
    set_copy(SET(self), SET(parent));
}
static LK_OBJ_DEFMARKFUNC(mark__gset) {
    SET_EACH(SET(self), i, mark(LK_OBJ(i->key)));
}
static LK_OBJ_DEFFREEFUNC(free__gset) {
    set_fin(SET(self));
}
LK_EXT_DEFINIT(lk_Gset_extinittypes) {
    vm->t_gset = lk_Object_allocwithsize(vm->t_obj, sizeof(lk_Gset_t));
    set_init(SET(vm->t_gset), 0, lk_Object_hashcode, lk_Object_keycmp);
    lk_Object_setallocfunc(vm->t_gset, alloc__gset);
    lk_Object_setmarkfunc(vm->t_gset, mark__gset);
    lk_Object_setfreefunc(vm->t_gset, free__gset);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(clearB__gset) {
    set_clear(SET(self));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(count__gset) {
    RETURN(lk_Fi_new(VM, set_count(SET(self))));
}
LK_LIBRARY_DEFINECFUNCTION(keys__gset) {
    lk_List_t *keys = lk_List_new(VM);
    SET_EACH(SET(self), i, Sequence_pushptr(LIST(keys), (void *)i->key));
    RETURN(keys);
}
LK_EXT_DEFINIT(lk_Gset_extinitfuncs) {
    lk_Object_t *gset = vm->t_gset;
    lk_Library_setGlobal("GenericSet", gset);
    lk_Library_setCFunction(gset, "clear!", clearB__gset, NULL);
    lk_Library_setCFunction(gset, "count", count__gset, NULL);
    lk_Library_setCFunction(gset, "keys", keys__gset, NULL);
}
