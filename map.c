#include "ext.h"
#include "fixnum.h"
#include "map.h"
#include "obj.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__map) {
    set_copy(SET(self), SET(parent));
}
static LK_OBJ_DEFMARKFUNC(mark__map) {
    SET_EACH(SET(self), i,
        mark(LK_OBJ(i->key));
        mark(SETITEM_VALUE(lk_object_t *, i));
    );
}
static LK_OBJ_DEFFREEFUNC(free__map) {
    set_fin(SET(self));
}
LK_EXT_DEFINIT(lk_map_extinittypes) {
    vm->t_map = lk_object_allocwithsize(vm->t_obj, sizeof(lk_map_t));
    set_init(SET(vm->t_map), sizeof(lk_object_t *), lk_object_hashcode, lk_object_keycmp);
    lk_object_setallocfunc(vm->t_map, alloc__map);
    lk_object_setmarkfunc(vm->t_map, mark__map);
    lk_object_setfreefunc(vm->t_map, free__map);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(at__map_obj) {
    setitem_t *i = set_get(SET(self), ARG(0));
    RETURN(i != NULL ? SETITEM_VALUE(lk_object_t *, i) : N);
}
LK_LIBRARY_DEFINECFUNCTION(clearB__map) {
    set_clear(SET(self));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(count__map) {
    RETURN(lk_fi_new(VM, set_count(SET(self))));
}
LK_LIBRARY_DEFINECFUNCTION(keys__map) {
    lk_list_t *keys = lk_list_new(VM);
    SET_EACH(SET(self), i, array_pushptr(LIST(keys), (void *)i->key));
    RETURN(keys);
}
LK_LIBRARY_DEFINECFUNCTION(setB__map_obj_obj) {
    *(lk_object_t **)set_set(SET(self),
    lk_object_addref(self, ARG(0))) = lk_object_addref(self, ARG(1));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(values__map) {
    lk_list_t *values = lk_list_new(VM);
    SET_EACH(SET(self), i,
        array_pushptr(LIST(values), SETITEM_VALUE(lk_object_t *, i));
    );
    RETURN(values);
}
LK_EXT_DEFINIT(lk_map_extinitfuncs) {
    lk_object_t *map = vm->t_map, *obj = vm->t_obj;
    lk_library_setGlobal("Map", map);
    lk_library_setCFunction(map, "at", at__map_obj, obj, NULL);
    lk_library_setCFunction(map, "clear!", clearB__map, NULL);
    lk_library_setCFunction(map, "count", count__map, NULL);
    lk_library_setCFunction(map, "keys", keys__map, NULL);
    lk_library_setCFunction(map, "set!", setB__map_obj_obj, obj, obj, NULL);
    lk_library_setCFunction(map, "values", values__map, NULL);
}

/* new */
lk_map_t *lk_map_new(lk_vm_t *vm) {
    return LK_MAP(lk_object_alloc(vm->t_map));
}
lk_map_t *lk_map_newfromset(lk_vm_t *vm, set_t *from) {
    lk_map_t *self = lk_map_new(vm);
    set_copy(SET(self), from);
    return self;
}

/* update */
void lk_map_set(lk_map_t *self, lk_object_t *k, lk_object_t *v) {
    *(lk_object_t **)set_set(SET(self),
    lk_object_addref(LK_OBJ(self), k)) = lk_object_addref(LK_OBJ(self), v);
}
void lk_map_setbycstr(lk_map_t *self, const char *k, lk_object_t *v) {
    lk_map_set(self, LK_OBJ(lk_string_newfromcstr(LK_VM(self), k)), v);
}

/* info */
lk_object_t *lk_map_get(lk_map_t *self, lk_object_t *k) {
    setitem_t *i = set_get(SET(self), k);
    return i != NULL ? SETITEM_VALUE(lk_object_t *, i) : NULL;
}
lk_object_t *lk_map_getbycstr(lk_map_t *self, const char *k) {
    return lk_map_get(self, LK_OBJ(lk_string_newfromcstr(LK_VM(self), k)));
}
