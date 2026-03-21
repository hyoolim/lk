#ifndef LK_OBJ_H
#define LK_OBJ_H
#include "types.h"

/* type */
struct lk_obj {
    struct lk_common o;
};
enum lk_slottype {
    LK_SLOTTYPE_LKOBJ,
    LK_SLOTTYPE_CFIELDLKOBJ
};
enum lk_slotoption {
    LK_SLOTOPTION_READONLY = 1 << 4,
    LK_SLOTOPTION_AUTOSEND = 1 << 5
};
struct lk_slot {
    enum lk_slottype  typeandoption;
    lk_obj_t      *check;
    union {
        lk_obj_t  *lkobj;
        size_t        coffset;
    }                 value;
};
#define LK_SLOT(v) ((struct lk_slot *)(v))
#define LK_SLOT_TYPEMASK ((1 << 4) - 1)
#define LK_SLOT_OPTIONMASK (~LK_SLOT_TYPEMASK)
#define LK_SLOT_GETTYPE(self) ((self)->typeandoption & LK_SLOT_TYPEMASK)
#define LK_SLOT_SETTYPE(self, type) ((self)->typeandoption = ((self)->typeandoption & LK_SLOT_OPTIONMASK) | (type))
#define LK_SLOT_SETOPTION(self, option) ((self)->typeandoption |= (option))
#define LK_SLOT_CHECKOPTION(self, option) ((self)->typeandoption & (option))

/* ext map */
void lk_obj_typeinit(lk_vm_t *vm);
void lk_obj_libinit(lk_vm_t *vm);

/* new */
#define LK_OBJ_MAXRECYCLED 1000
lk_obj_t *lk_obj_alloc_withsize(lk_obj_t *parent, size_t s);
lk_obj_t *lk_obj_alloc(lk_obj_t *parent);
lk_obj_t *lk_obj_clone(lk_obj_t *self);
void lk_obj_justfree(lk_obj_t *self);
void lk_obj_free(lk_obj_t *self);

/* update - tag */
#define LK_OBJ_DEFTAGSETTER(t, field) \
void lk_obj_set ## field(lk_obj_t *self, t field)
LK_OBJ_DEFTAGSETTER(lk_tagallocfunc_t *, allocfunc);
LK_OBJ_DEFTAGSETTER(lk_tagmarkfunc_t *, markfunc);
LK_OBJ_DEFTAGSETTER(lk_tagfreefunc_t *, freefunc);

/* update */
void lk_obj_extend(lk_obj_t *self, lk_obj_t *parent);
struct lk_slot *lk_obj_setslot(lk_obj_t *self, lk_obj_t *k, lk_obj_t *check, lk_obj_t *v);
struct lk_slot *lk_obj_setslotbycstr(lk_obj_t *self, const char *k, lk_obj_t *check, lk_obj_t *v);
void lk_obj_setvalueonslot(lk_obj_t *self, struct lk_slot *slot, lk_obj_t *v);
int lk_obj_calcancestors(lk_obj_t *self);
void lk_obj_set_cfunc_lk(lk_obj_t *self, const char *name, lk_cfunc_lk_t *cfunc, ...);
void lk_obj_set_cfunc_creturn(lk_obj_t *self, const char *name, ...);
void lk_obj_set_cfunc_cvoid(lk_obj_t *self, const char *name, ...);

/* info */
int lk_obj_isa(lk_obj_t *self, lk_obj_t *t);
struct lk_slot *lk_obj_getslot(lk_obj_t *self, lk_obj_t *k);
struct lk_slot *lk_obj_getslotfromany(lk_obj_t *self, lk_obj_t *k);
lk_obj_t *lk_obj_getvaluefromslot(lk_obj_t *self,
                                        struct lk_slot *slot);
int lk_obj_hashcode(const void *k, int cap);
int lk_obj_keycmp(const void *self, const void *other);
#define LK_OBJ_ISTYPE(self, t) \
(  (self) == (t) \
|| (t) == LK_VM(self)->t_obj \
/* || (  (t)->o.tag != LK_VM(self)->t_obj->o.tag \
   && (self)->o.tag == (t)->o.tag \
   ) */ \
|| lk_obj_isa((self), (t)) \
)
/*
#define LK_OBJ_ISA(self, t) ((self) == (t) ? 1 : \
LK_OBJ_HASONEPARENT((self)->o.parents) && \
LK_OBJ_ONEPARENT((self)->o.parents) == (t) ? 2 : \
lk_obj_isa((self), (t)))
#define LK_OBJ_HASONEPARENT(pars) ((ptrdiff_t)(pars) & 1)
#define LK_OBJ_ONEPARENT(pars) LK_OBJ((ptrdiff_t)(pars) & ~1)
#define LK_OBJ_PROTO(self) LK_OBJ_HASONEPARENT((self)->o.parents) \
? LK_OBJ_ONEPARENT((self)->o.parents) : DARRAY_ATPTR((self)->o.parents, 0)
 */
#define LK_OBJ_HASPARENTS(self) ((ptrdiff_t)((self)->o.parent) & 1)
#define LK_OBJ_PARENTS(self) ((darray_t *)((ptrdiff_t)((self)->o.parent) & ~1))
#define LK_OBJ_PROTO(self) ( \
    LK_OBJ_HASPARENTS(self) \
    ? darray_ptr_get(LK_OBJ_PARENTS(self), -1) \
    : (self)->o.parent \
)
#define LK_OBJ_ISA(self, t) ( \
    (self) == (t) ? 1 : \
    !LK_OBJ_HASPARENTS(self) && (self)->o.parent == (t) ? 2 : \
    lk_obj_isa((self), (t)) \
)
#define LK_OBJ_ISCFUNC(self) ( \
    (self)->o.tag->allocfunc == LK_VM(self)->t_cfunc->o.tag->allocfunc)
#define LK_OBJ_ISSCOPE(self) ( \
    (self)->o.tag->freefunc == LK_VM(self)->t_scope->o.tag->freefunc)
#define LK_OBJ_ISGFUNC(self) ( \
    (self)->o.tag->freefunc == LK_VM(self)->t_gfunc->o.tag->freefunc)
#define LK_OBJ_ISFUNC(self) ( \
    (self)->o.tag->freefunc == LK_VM(self)->t_func->o.tag->freefunc || \
    LK_OBJ_ISGFUNC(self) \
)
#define LK_OBJ_ISINSTR(self) ( \
    (self)->o.tag->markfunc == LK_VM(self)->t_instr->o.tag->markfunc)
#endif
