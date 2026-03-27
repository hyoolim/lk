#ifndef LK_OBJ_H
#define LK_OBJ_H
#include "types.h"

// type
struct lk_obj {
    struct lk_common o;
};
enum lk_slottype { LK_SLOTTYPE_LKOBJ, LK_SLOTTYPE_CFIELDLKOBJ };
enum lk_slotoption { LK_SLOTOPTION_READONLY = 1 << 4, LK_SLOTOPTION_AUTOSEND = 1 << 5 };
struct lk_slot {
    enum lk_slottype typeandoption;
    lk_obj_t *check;
    union {
        lk_obj_t *lkobj;
        size_t coffset;
    } value;
};
#define LK_SLOT(v) ((struct lk_slot *)(v))
#define LK_SLOT_TYPEMASK ((1 << 4) - 1)
#define LK_SLOT_OPTIONMASK (~LK_SLOT_TYPEMASK)
#define LK_SLOT_GETTYPE(self) ((self)->typeandoption & LK_SLOT_TYPEMASK)
#define LK_SLOT_SETTYPE(self, type) ((self)->typeandoption = ((self)->typeandoption & LK_SLOT_OPTIONMASK) | (type))
#define LK_SLOT_SETOPTION(self, option) ((self)->typeandoption |= (option))
#define LK_SLOT_CHECKOPTION(self, option) ((self)->typeandoption & (option))

// ext map
void lk_obj_type_init(lk_vm_t *vm);
void lk_obj_lib_init(lk_vm_t *vm);

// new
#define LK_OBJ_MAXRECYCLED 1000
lk_obj_t *lk_obj_alloc_type(lk_obj_t *parent, size_t s);
lk_obj_t *lk_obj_alloc(lk_obj_t *parent);
lk_obj_t *lk_obj_clone(lk_obj_t *self);
void lk_obj_just_free(lk_obj_t *self);
void lk_obj_free(lk_obj_t *self);

// update - view
#define LK_OBJ_DEFVIEWSETTER(t, field) void lk_obj_set_##field(lk_obj_t *self, t field)
LK_OBJ_DEFVIEWSETTER(lk_viewallocfunc_t *, alloc_func);
LK_OBJ_DEFVIEWSETTER(lk_viewmarkfunc_t *, mark_func);
LK_OBJ_DEFVIEWSETTER(lk_viewfreefunc_t *, free_func);

// update
void lk_obj_extend(lk_obj_t *self, lk_obj_t *parent);
struct lk_slot *lk_obj_setslot(lk_obj_t *self, lk_obj_t *k, lk_obj_t *check, lk_obj_t *v);
struct lk_slot *lk_obj_set_slot_by_cstr(lk_obj_t *self, const char *k, lk_obj_t *check, lk_obj_t *v);
lk_obj_t *lk_obj_get_value_by_cstr(lk_obj_t *self, const char *k);
void lk_obj_set_value_on_slot(lk_obj_t *self, struct lk_slot *slot, lk_obj_t *v);
int lk_obj_calc_ancestors(lk_obj_t *self);
void lk_obj_set_cfunc_lk(lk_obj_t *self, const char *name, lk_cfunc_lk_t *cfunc, ...);
void lk_obj_set_cfunc_creturn(lk_obj_t *self, const char *name, ...);
void lk_obj_set_cfunc_cvoid(lk_obj_t *self, const char *name, ...);

// info
int lk_obj_isa(lk_obj_t *self, lk_obj_t *t);
struct lk_slot *lk_obj_getslot(lk_obj_t *self, lk_obj_t *k);
struct lk_slot *lk_obj_get_slot_from_any(lk_obj_t *self, lk_obj_t *k);
lk_obj_t *lk_obj_get_value_from_slot(lk_obj_t *self, struct lk_slot *slot);
int lk_obj_hash_code(const void *k, int cap);
int lk_obj_key_cmp(const void *self, const void *other);
#define LK_OBJ_ISTYPE(self, t) \
    ((self) == (t) || (t) == LK_VM(self)->t_obj /* || (  (t)->o.view != LK_VM(self)->t_obj->o.view \
                                                   && (self)->o.view == (t)->o.view \
                                                   ) */ \
     || lk_obj_isa((self), (t)))
/*
#define LK_OBJ_ISA(self, t) ((self) == (t) ? 1 : \
LK_OBJ_HASONEPARENT((self)->o.parents) && \
LK_OBJ_ONEPARENT((self)->o.parents) == (t) ? 2 : \
lk_obj_isa((self), (t)))
#define LK_OBJ_HASONEPARENT(pars) ((ptrdiff_t)(pars) & 1)
#define LK_OBJ_ONEPARENT(pars) LK_OBJ((ptrdiff_t)(pars) & ~1)
#define LK_OBJ_PROTO(self) LK_OBJ_HASONEPARENT((self)->o.parents) \
? LK_OBJ_ONEPARENT((self)->o.parents) : VEC_ATPTR((self)->o.parents, 0)
 */
#define LK_OBJ_HASPARENTS(self) ((ptrdiff_t)((self)->o.view->parent) & 1)
#define LK_OBJ_PARENTS(self) ((vec_t *)((ptrdiff_t)((self)->o.view->parent) & ~1))
#define LK_OBJ_PROTO(self) (LK_OBJ_HASPARENTS(self) ? vec_ptr_get(LK_OBJ_PARENTS(self), -1) : (self)->o.view->parent)
#define LK_OBJ_ISA(self, t) \
    ((self) == (t) ? 1 : !LK_OBJ_HASPARENTS(self) && (self)->o.view->parent == (t) ? 2 : lk_obj_isa((self), (t)))
#define LK_OBJ_ISCFUNC(self) ((self)->o.view->alloc_func == LK_VM(self)->t_cfunc->o.view->alloc_func)
#define LK_OBJ_ISSCOPE(self) ((self)->o.view->free_func == LK_VM(self)->t_scope->o.view->free_func)
#define LK_OBJ_ISGFUNC(self) ((self)->o.view->free_func == LK_VM(self)->t_gfunc->o.view->free_func)
#define LK_OBJ_ISFUNC(self) \
    ((self)->o.view->free_func == LK_VM(self)->t_func->o.view->free_func || LK_OBJ_ISGFUNC(self))
#define LK_OBJ_ISINSTR(self) ((self)->o.view->mark_func == LK_VM(self)->t_instr->o.view->mark_func)
#endif
