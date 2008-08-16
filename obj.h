#ifndef LK_OBJ_H
#define LK_OBJ_H
#include "vm.h"

/* type */
enum lk_Slottype {
    LK_SLOTTYPE_LKOBJ,
    LK_SLOTTYPE_CFIELDLKOBJ
};
enum lk_Slotoption {
    LK_SLOTOPTION_READONLY = 1 << 4,
    LK_SLOTOPTION_AUTOSEND = 1 << 5
};
struct lk_Slot {
    enum lk_Slottype  typeandoption;
    lk_Object_t      *check;
    union {
        lk_Object_t  *lkobj;
        size_t        coffset;
    }                 value;
};
#define LK_SLOT(v) ((struct lk_Slot *)(v))
#define LK_SLOT_TYPEMASK ((1 << 4) - 1)
#define LK_SLOT_OPTIONMASK (~LK_SLOT_TYPEMASK)
#define LK_SLOT_GETTYPE(self) ((self)->typeandoption & LK_SLOT_TYPEMASK)
#define LK_SLOT_SETTYPE(self, type) ((self)->typeandoption = ((self)->typeandoption & LK_SLOT_OPTIONMASK) | (type))
#define LK_SLOT_SETOPTION(self, option) ((self)->typeandoption |= (option))
#define LK_SLOT_CHECKOPTION(self, option) ((self)->typeandoption & (option))

/* ext map */
LK_EXT_DEFINIT(lk_Object_extinittypes);
LK_EXT_DEFINIT(lk_Object_extinitfuncs);

/* new */
#define LK_OBJ_MAXRECYCLED 1000
lk_Object_t *lk_Object_allocwithsize(lk_Object_t *parent, size_t s);
lk_Object_t *lk_Object_alloc(lk_Object_t *parent);
lk_Object_t *lk_Object_clone(lk_Object_t *self);
void lk_Object_justfree(lk_Object_t *self);
void lk_Object_free(lk_Object_t *self);

/* update - tag */
#define LK_OBJ_DEFTAGSETTER(t, field) \
void lk_Object_set ## field(lk_Object_t *self, t field)
LK_OBJ_DEFTAGSETTER(lk_Tagallocfunc_t *, allocfunc);
LK_OBJ_DEFTAGSETTER(lk_Tagmarkfunc_t *, markfunc);
LK_OBJ_DEFTAGSETTER(lk_Tagfreefunc_t *, freefunc);

/* update */
struct lk_Slot *lk_Object_setslot(lk_Object_t *self, lk_Object_t *k,
                                  lk_Object_t *check, lk_Object_t *v);
struct lk_Slot *lk_Object_setslotbycstr(lk_Object_t *self, const char *k,
                                        lk_Object_t *check, lk_Object_t *v);
void lk_Object_setvalueonslot(lk_Object_t *self, struct lk_Slot *slot,
                              lk_Object_t *v);
int lk_Object_calcancestors(lk_Object_t *self);

/* info */
int lk_Object_isa(lk_Object_t *self, lk_Object_t *t);
struct lk_Slot *lk_Object_getslot(lk_Object_t *self, lk_Object_t *k);
struct lk_Slot *lk_Object_getslotfromany(lk_Object_t *self, lk_Object_t *k);
lk_Object_t *lk_Object_getvaluefromslot(lk_Object_t *self,
                                        struct lk_Slot *slot);
int lk_Object_hashcode(const void *k, int capa);
int lk_Object_keycmp(const void *self, const void *other);
#define LK_OBJ_ISTYPE(self, t) \
(  (self) == (t) \
|| (t) == LK_VM(self)->t_obj \
/* || (  (t)->obj.tag != LK_VM(self)->t_obj->obj.tag \
   && (self)->obj.tag == (t)->obj.tag \
   ) */ \
|| lk_Object_isa((self), (t)) \
)
/*
#define LK_OBJ_ISA(self, t) ((self) == (t) ? 1 : \
LK_OBJ_HASONEPARENT((self)->obj.parents) && \
LK_OBJ_ONEPARENT((self)->obj.parents) == (t) ? 2 : \
lk_Object_isa((self), (t)))
#define LK_OBJ_HASONEPARENT(pars) ((ptrdiff_t)(pars) & 1)
#define LK_OBJ_ONEPARENT(pars) LK_OBJ((ptrdiff_t)(pars) & ~1)
#define LK_OBJ_PROTO(self) LK_OBJ_HASONEPARENT((self)->obj.parents) \
? LK_OBJ_ONEPARENT((self)->obj.parents) : LIST_ATPTR((self)->obj.parents, 0)
 */
#define LK_OBJ_HASPARENTS(self) ((ptrdiff_t)((self)->obj.parent) & 1)
#define LK_OBJ_PARENTS(self) ((Sequence_t *)((ptrdiff_t)((self)->obj.parent) & ~1))
#define LK_OBJ_PROTO(self) ( \
    LK_OBJ_HASPARENTS(self) \
    ? Sequence_getptr(LK_OBJ_PARENTS(self), -1) \
    : (self)->obj.parent \
)
#define LK_OBJ_ISA(self, t) ( \
    (self) == (t) ? 1 : \
    !LK_OBJ_HASPARENTS(self) && (self)->obj.parent == (t) ? 2 : \
    lk_Object_isa((self), (t)) \
)
#define LK_OBJ_ISCFUNC(self) ( \
    (self)->obj.tag->allocfunc == LK_VM(self)->t_cfunc->obj.tag->allocfunc)
#define LK_OBJ_ISFRAME(self) ( \
    (self)->obj.tag->freefunc == LK_VM(self)->t_frame->obj.tag->freefunc)
#define LK_OBJ_ISGFUNC(self) ( \
    (self)->obj.tag->freefunc == LK_VM(self)->t_gfunc->obj.tag->freefunc)
#define LK_OBJ_ISFUNC(self) ( \
    (self)->obj.tag->freefunc == LK_VM(self)->t_func->obj.tag->freefunc || \
    LK_OBJ_ISGFUNC(self) \
)
#define LK_OBJ_ISINSTR(self) ( \
    (self)->obj.tag->markfunc == LK_VM(self)->t_instr->obj.tag->markfunc)
#endif
