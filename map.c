#include "ext.h"
#include "number.h"
#include "map.h"
#include "object.h"

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc_map) {
    qphash_copy(QPHASH(self), QPHASH(parent));
}
static LK_OBJ_DEFMARKFUNC(mark_map) {
    SET_EACH(QPHASH(self), i,
        mark(LK_OBJ(i->key));
        mark(SETITEM_VALUE(lk_object_t *, i));
    );
}
static LK_OBJ_DEFFREEFUNC(free_map) {
    qphash_fin(QPHASH(self));
}
LK_LIB_DEFINEINIT(lk_map_libPreInit) {
    vm->t_map = lk_object_allocWithSize(vm->t_object, sizeof(lk_map_t));
    qphash_init(QPHASH(vm->t_map), sizeof(lk_object_t *), lk_object_hashcode, lk_object_keycmp);
    lk_object_setallocfunc(vm->t_map, alloc_map);
    lk_object_setmarkfunc(vm->t_map, mark_map);
    lk_object_setfreefunc(vm->t_map, free_map);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(at_map_obj) {
    setitem_t *i = qphash_get(QPHASH(self), ARG(0));
    RETURN(i != NULL ? SETITEM_VALUE(lk_object_t *, i) : NIL);
}
LK_LIB_DEFINECFUNC(clearB_map) {
    qphash_clear(QPHASH(self));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(size_map) {
    RETURN(lk_number_new(VM, qphash_size(QPHASH(self))));
}
LK_LIB_DEFINECFUNC(keys_map) {
    lk_list_t *keys = lk_list_new(VM);
    SET_EACH(QPHASH(self), i, darray_pushptr(DARRAY(keys), (void *)i->key));
    RETURN(keys);
}
LK_LIB_DEFINECFUNC(setB_map_obj_obj) {
    *(lk_object_t **)qphash_set(QPHASH(self),
    lk_object_addref(self, ARG(0))) = lk_object_addref(self, ARG(1));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(values_map) {
    lk_list_t *values = lk_list_new(VM);
    SET_EACH(QPHASH(self), i,
        darray_pushptr(DARRAY(values), SETITEM_VALUE(lk_object_t *, i));
    );
    RETURN(values);
}
LK_LIB_DEFINEINIT(lk_map_libInit) {
    lk_object_t *map = vm->t_map, *obj = vm->t_object;
    lk_lib_setGlobal("Map", map);
    lk_lib_setCFunc(map, "at", at_map_obj, obj, NULL);
    lk_lib_setCFunc(map, "clear!", clearB_map, NULL);
    lk_lib_setCFunc(map, "size", size_map, NULL);
    lk_lib_setCFunc(map, "keys", keys_map, NULL);
    lk_lib_setCFunc(map, "set!", setB_map_obj_obj, obj, obj, NULL);
    lk_lib_setCFunc(map, "values", values_map, NULL);
}

/* new */
lk_map_t *lk_map_new(lk_vm_t *vm) {
    return LK_MAP(lk_object_alloc(vm->t_map));
}
lk_map_t *lk_map_newfromset(lk_vm_t *vm, qphash_t *from) {
    lk_map_t *self = lk_map_new(vm);
    qphash_copy(QPHASH(self), from);
    return self;
}

/* update */
void lk_map_set(lk_map_t *self, lk_object_t *k, lk_object_t *v) {
    *(lk_object_t **)qphash_set(QPHASH(self),
    lk_object_addref(LK_OBJ(self), k)) = lk_object_addref(LK_OBJ(self), v);
}
void lk_map_setWithCStringKey(lk_map_t *self, const char *k, lk_object_t *v) {
    lk_map_set(self, LK_OBJ(lk_string_newFromCString(LK_VM(self), k)), v);
}

/* info */
lk_object_t *lk_map_get(lk_map_t *self, lk_object_t *k) {
    setitem_t *i = qphash_get(QPHASH(self), k);
    return i != NULL ? SETITEM_VALUE(lk_object_t *, i) : NULL;
}
lk_object_t *lk_map_getByCStringKey(lk_map_t *self, const char *k) {
    return lk_map_get(self, LK_OBJ(lk_string_newFromCString(LK_VM(self), k)));
}
