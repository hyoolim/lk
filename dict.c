#include "dict.h"
#include "ext.h"
#include "fixnum.h"
#include "object.h"

/* ext map - types */
static LK_OBJECT_DEFMARKFUNC(mark__dict) {
    PT_SET_EACH(SET(self), i,
        mark(LK_O(i->key));
        mark(PT_SETITEM_VALUE(lk_object_t *, i));
    );
}
LK_EXT_DEFINIT(lk_dict_extinittypes) {
    vm->t_dict = lk_object_alloc(vm->t_gset);
    pt_set_fin(SET(vm->t_dict));
    pt_set_init(SET(vm->t_dict), sizeof(lk_object_t *),
                lk_object_hashcode, lk_object_keycmp);
    lk_object_setmarkfunc(vm->t_dict, mark__dict);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(at__dict_obj) {
    pt_setitem_t *i = pt_set_get(SET(self), ARG(0));
    RETURN(i != NULL ? PT_SETITEM_VALUE(lk_object_t *, i) : N);
}
static LK_EXT_DEFCFUNC(setB__dict_obj_obj) {
    *(lk_object_t **)pt_set_set(SET(self),
    lk_object_addref(self, ARG(0))) = lk_object_addref(self, ARG(1));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(values__dict) {
    lk_list_t *values = lk_list_new(VM);
    PT_SET_EACH(SET(self), i,
        pt_list_pushptr(LIST(values), PT_SETITEM_VALUE(lk_object_t *, i));
    );
    RETURN(values);
}
LK_EXT_DEFINIT(lk_dict_extinitfuncs) {
    lk_object_t *dict = vm->t_dict, *obj = vm->t_object;
    lk_ext_global("Dictionary", dict);
    lk_ext_cfunc(dict, "at", at__dict_obj, obj, NULL);
    lk_ext_cfunc(dict, "set!", setB__dict_obj_obj, obj, obj, NULL);
    lk_ext_cfunc(dict, "values", values__dict, NULL);
}

/* new */
lk_dict_t *lk_dict_new(lk_vm_t *vm) {
    return LK_DICT(lk_object_alloc(vm->t_dict));
}
lk_dict_t *lk_dict_newfromset(lk_vm_t *vm, pt_set_t *from) {
    lk_dict_t *self = lk_dict_new(vm);
    pt_set_copy(SET(self), from);
    return self;
}

/* update */
void lk_dict_set(lk_dict_t *self, lk_object_t *k, lk_object_t *v) {
    *(lk_object_t **)pt_set_set(SET(self),
    lk_object_addref(LK_O(self), k)) = lk_object_addref(LK_O(self), v);
}
void lk_dict_setbycstr(lk_dict_t *self, const char *k, lk_object_t *v) {
    lk_dict_set(self, LK_O(lk_string_newfromcstr(LK_VM(self), k)), v);
}

/* info */
lk_object_t *lk_dict_get(lk_dict_t *self, lk_object_t *k) {
    pt_setitem_t *i = pt_set_get(SET(self), k);
    return i != NULL ? PT_SETITEM_VALUE(lk_object_t *, i) : NULL;
}
lk_object_t *lk_dict_getbycstr(lk_dict_t *self, const char *k) {
    return lk_dict_get(self, LK_O(lk_string_newfromcstr(LK_VM(self), k)));
}
