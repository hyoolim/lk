#include "dict.h"
#include "ext.h"
#include "fixnum.h"
#include "obj.h"

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark__dict) {
    SET_EACH(SET(self), i,
        mark(LK_OBJ(i->key));
        mark(SETITEM_VALUE(lk_obj_t *, i));
    );
}
LK_EXT_DEFINIT(lk_dict_extinittypes) {
    vm->t_dict = lk_obj_alloc(vm->t_gset);
    set_fin(SET(vm->t_dict));
    set_init(SET(vm->t_dict), sizeof(lk_obj_t *),
                lk_obj_hashcode, lk_obj_keycmp);
    lk_obj_setmarkfunc(vm->t_dict, mark__dict);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(at__dict_obj) {
    setitem_t *i = set_get(SET(self), ARG(0));
    RETURN(i != NULL ? SETITEM_VALUE(lk_obj_t *, i) : N);
}
static LK_EXT_DEFCFUNC(setB__dict_obj_obj) {
    *(lk_obj_t **)set_set(SET(self),
    lk_obj_addref(self, ARG(0))) = lk_obj_addref(self, ARG(1));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(values__dict) {
    lk_list_t *values = lk_list_new(VM);
    SET_EACH(SET(self), i,
        list_pushptr(LIST(values), SETITEM_VALUE(lk_obj_t *, i));
    );
    RETURN(values);
}
LK_EXT_DEFINIT(lk_dict_extinitfuncs) {
    lk_obj_t *dict = vm->t_dict, *obj = vm->t_obj;
    lk_ext_global("Dictionary", dict);
    lk_ext_cfunc(dict, "at", at__dict_obj, obj, NULL);
    lk_ext_cfunc(dict, "set!", setB__dict_obj_obj, obj, obj, NULL);
    lk_ext_cfunc(dict, "values", values__dict, NULL);
}

/* new */
lk_dict_t *lk_dict_new(lk_vm_t *vm) {
    return LK_DICT(lk_obj_alloc(vm->t_dict));
}
lk_dict_t *lk_dict_newfromset(lk_vm_t *vm, set_t *from) {
    lk_dict_t *self = lk_dict_new(vm);
    set_copy(SET(self), from);
    return self;
}

/* update */
void lk_dict_set(lk_dict_t *self, lk_obj_t *k, lk_obj_t *v) {
    *(lk_obj_t **)set_set(SET(self),
    lk_obj_addref(LK_OBJ(self), k)) = lk_obj_addref(LK_OBJ(self), v);
}
void lk_dict_setbycstr(lk_dict_t *self, const char *k, lk_obj_t *v) {
    lk_dict_set(self, LK_OBJ(lk_string_newfromcstr(LK_VM(self), k)), v);
}

/* info */
lk_obj_t *lk_dict_get(lk_dict_t *self, lk_obj_t *k) {
    setitem_t *i = set_get(SET(self), k);
    return i != NULL ? SETITEM_VALUE(lk_obj_t *, i) : NULL;
}
lk_obj_t *lk_dict_getbycstr(lk_dict_t *self, const char *k) {
    return lk_dict_get(self, LK_OBJ(lk_string_newfromcstr(LK_VM(self), k)));
}
