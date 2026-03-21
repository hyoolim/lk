#include "lib.h"

/* type */
static void alloc_map(lk_obj_t *self, lk_obj_t *parent) {
    qphash_copy(QPHASH(self), QPHASH(parent));
}
static LK_OBJ_DEFMARKFUNC(mark_map) {
    SET_EACH(QPHASH(self), i,
        mark(LK_OBJ(i->key));
        mark(SETITEM_VALUE(lk_obj_t *, i));
    );
}
static void free_map(lk_obj_t *self) {
    qphash_fin(QPHASH(self));
}
void lk_map_typeinit(lk_vm_t *vm) {
    vm->t_map = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_map_t));
    qphash_init(QPHASH(vm->t_map), sizeof(lk_obj_t *), lk_obj_hashcode, lk_obj_keycmp);
    lk_obj_setallocfunc(vm->t_map, alloc_map);
    lk_obj_setmarkfunc(vm->t_map, mark_map);
    lk_obj_setfreefunc(vm->t_map, free_map);
}

/* new */
lk_map_t *lk_map_new(lk_vm_t *vm) {
    return LK_MAP(lk_obj_alloc(vm->t_map));
}
lk_map_t *lk_map_newfromset(lk_vm_t *vm, qphash_t *from) {
    lk_map_t *self = lk_map_new(vm);
    qphash_copy(QPHASH(self), from);
    return self;
}

/* update */
void lk_map_clear(lk_map_t *self) {
    qphash_clear(QPHASH(self));
}
void lk_map_set(lk_map_t *self, lk_obj_t *k, lk_obj_t *v) {
    *(lk_obj_t **)qphash_set(QPHASH(self),
    lk_obj_addref(LK_OBJ(self), k)) = lk_obj_addref(LK_OBJ(self), v);
}
void lk_map_set_str_obj(lk_map_t *self, lk_str_t *key, lk_obj_t *value) {
    *(lk_obj_t **)qphash_set(QPHASH(self), lk_obj_addref(LK_OBJ(self), LK_OBJ(key))) = lk_obj_addref(LK_OBJ(self), value);
}
void lk_map_setWithCStringKey(lk_map_t *self, const char *k, lk_obj_t *v) {
    lk_map_set(self, LK_OBJ(lk_str_new_fromcstr(LK_VM(self), k)), v);
}

/* info */
lk_obj_t *lk_map_at_str(lk_map_t *self, lk_str_t *key) {
    setitem_t *item = qphash_get(QPHASH(self), key);
    return item != NULL ? SETITEM_VALUE(lk_obj_t *, item) : NIL;
}
lk_obj_t *lk_map_get(lk_map_t *self, lk_obj_t *k) {
    setitem_t *i = qphash_get(QPHASH(self), k);
    return i != NULL ? SETITEM_VALUE(lk_obj_t *, i) : NULL;
}
lk_obj_t *lk_map_getByCStringKey(lk_map_t *self, const char *k) {
    return lk_map_get(self, LK_OBJ(lk_str_new_fromcstr(LK_VM(self), k)));
}
lk_list_t *lk_map_keys(lk_map_t *self) {
    lk_list_t *keys = lk_list_new(VM);
    SET_EACH(QPHASH(self), i, darray_ptr_push(DARRAY(keys), (void *)i->key));
    return keys;
}
lk_num_t *lk_map_size(lk_map_t *self) {
    return lk_num_new(VM, qphash_size(QPHASH(self)));
}
lk_list_t *lk_map_values(lk_map_t *self) {
    lk_list_t *values = lk_list_new(VM);
    SET_EACH(QPHASH(self), i, darray_ptr_push(DARRAY(values), SETITEM_VALUE(lk_obj_t *, i)));
    return values;
}

/* bind all c funcs to lk equiv */
void lk_map_libinit(lk_vm_t *vm) {
    lk_obj_t *map = vm->t_map, *obj = vm->t_obj, *str = vm->t_str;
    lk_global_set("Map", map);

    /* update */
    lk_obj_set_cfunc_cvoid(map, "clear!", lk_map_clear, NULL);
    lk_obj_set_cfunc_cvoid(map, "set!", lk_map_set_str_obj, str, obj, NULL);

    /* info */
    lk_obj_set_cfunc_creturn(map, "at", lk_map_at_str, str, NULL);
    lk_obj_set_cfunc_creturn(map, "size", lk_map_size, NULL);
    lk_obj_set_cfunc_creturn(map, "keys", lk_map_keys, NULL);
    lk_obj_set_cfunc_creturn(map, "values", lk_map_values, NULL);
}
