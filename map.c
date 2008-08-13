#include "ext.h"
#include "fixnum.h"
#include "map.h"
#include "obj.h"

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark__map) {
    SET_EACH(SET(self), i,
        mark(LK_OBJ(i->key));
        mark(SETITEM_VALUE(lk_obj_t *, i));
    );
}
LK_EXT_DEFINIT(lk_map_extinittypes) {
    vm->t_map = lk_obj_alloc(vm->t_gset);
    set_fin(SET(vm->t_map));
    set_init(SET(vm->t_map), sizeof(lk_obj_t *),
                lk_obj_hashcode, lk_obj_keycmp);
    lk_obj_setmarkfunc(vm->t_map, mark__map);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(at__map_obj) {
    setitem_t *i = set_get(SET(self), ARG(0));
    RETURN(i != NULL ? SETITEM_VALUE(lk_obj_t *, i) : N);
}
static LK_EXT_DEFCFUNC(setB__map_obj_obj) {
    *(lk_obj_t **)set_set(SET(self),
    lk_obj_addref(self, ARG(0))) = lk_obj_addref(self, ARG(1));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(values__map) {
    lk_list_t *values = lk_list_new(VM);
    SET_EACH(SET(self), i,
        list_pushptr(LIST(values), SETITEM_VALUE(lk_obj_t *, i));
    );
    RETURN(values);
}
LK_EXT_DEFINIT(lk_map_extinitfuncs) {
    lk_obj_t *map = vm->t_map, *obj = vm->t_obj;
    lk_ext_global("Map", map);
    lk_ext_cfunc(map, "at", at__map_obj, obj, NULL);
    lk_ext_cfunc(map, "set!", setB__map_obj_obj, obj, obj, NULL);
    lk_ext_cfunc(map, "values", values__map, NULL);
}

/* new */
lk_map_t *lk_map_new(lk_vm_t *vm) {
    return LK_MAP(lk_obj_alloc(vm->t_map));
}
lk_map_t *lk_map_newfromset(lk_vm_t *vm, set_t *from) {
    lk_map_t *self = lk_map_new(vm);
    set_copy(SET(self), from);
    return self;
}

/* update */
void lk_map_set(lk_map_t *self, lk_obj_t *k, lk_obj_t *v) {
    *(lk_obj_t **)set_set(SET(self),
    lk_obj_addref(LK_OBJ(self), k)) = lk_obj_addref(LK_OBJ(self), v);
}
void lk_map_setbycstr(lk_map_t *self, const char *k, lk_obj_t *v) {
    lk_map_set(self, LK_OBJ(lk_string_newfromcstr(LK_VM(self), k)), v);
}

/* info */
lk_obj_t *lk_map_get(lk_map_t *self, lk_obj_t *k) {
    setitem_t *i = set_get(SET(self), k);
    return i != NULL ? SETITEM_VALUE(lk_obj_t *, i) : NULL;
}
lk_obj_t *lk_map_getbycstr(lk_map_t *self, const char *k) {
    return lk_map_get(self, LK_OBJ(lk_string_newfromcstr(LK_VM(self), k)));
}
