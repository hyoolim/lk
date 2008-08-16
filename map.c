#include "ext.h"
#include "fixnum.h"
#include "map.h"
#include "obj.h"

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark__map) {
    SET_EACH(SET(self), i,
        mark(LK_OBJ(i->key));
        mark(SETITEM_VALUE(lk_Object_t *, i));
    );
}
LK_EXT_DEFINIT(lk_Map_extinittypes) {
    vm->t_map = lk_Object_alloc(vm->t_gset);
    set_fin(SET(vm->t_map));
    set_init(SET(vm->t_map), sizeof(lk_Object_t *),
                lk_Object_hashcode, lk_Object_keycmp);
    lk_Object_setmarkfunc(vm->t_map, mark__map);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(at__map_obj) {
    setitem_t *i = set_get(SET(self), ARG(0));
    RETURN(i != NULL ? SETITEM_VALUE(lk_Object_t *, i) : N);
}
LK_LIBRARY_DEFINECFUNCTION(setB__map_obj_obj) {
    *(lk_Object_t **)set_set(SET(self),
    lk_Object_addref(self, ARG(0))) = lk_Object_addref(self, ARG(1));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(values__map) {
    lk_List_t *values = lk_List_new(VM);
    SET_EACH(SET(self), i,
        Sequence_pushptr(LIST(values), SETITEM_VALUE(lk_Object_t *, i));
    );
    RETURN(values);
}
LK_EXT_DEFINIT(lk_Map_extinitfuncs) {
    lk_Object_t *map = vm->t_map, *obj = vm->t_obj;
    lk_Library_setGlobal("Map", map);
    lk_Library_setCFunction(map, "at", at__map_obj, obj, NULL);
    lk_Library_setCFunction(map, "set!", setB__map_obj_obj, obj, obj, NULL);
    lk_Library_setCFunction(map, "values", values__map, NULL);
}

/* new */
lk_Map_t *lk_Map_new(lk_Vm_t *vm) {
    return LK_MAP(lk_Object_alloc(vm->t_map));
}
lk_Map_t *lk_Map_newfromset(lk_Vm_t *vm, set_t *from) {
    lk_Map_t *self = lk_Map_new(vm);
    set_copy(SET(self), from);
    return self;
}

/* update */
void lk_Map_set(lk_Map_t *self, lk_Object_t *k, lk_Object_t *v) {
    *(lk_Object_t **)set_set(SET(self),
    lk_Object_addref(LK_OBJ(self), k)) = lk_Object_addref(LK_OBJ(self), v);
}
void lk_Map_setbycstr(lk_Map_t *self, const char *k, lk_Object_t *v) {
    lk_Map_set(self, LK_OBJ(lk_String_newfromcstr(LK_VM(self), k)), v);
}

/* info */
lk_Object_t *lk_Map_get(lk_Map_t *self, lk_Object_t *k) {
    setitem_t *i = set_get(SET(self), k);
    return i != NULL ? SETITEM_VALUE(lk_Object_t *, i) : NULL;
}
lk_Object_t *lk_Map_getbycstr(lk_Map_t *self, const char *k) {
    return lk_Map_get(self, LK_OBJ(lk_String_newfromcstr(LK_VM(self), k)));
}
