#ifndef LK_OBJECT_H
#define LK_OBJECT_H
#include "vm.h"

/* type */
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
    lk_object_t      *check;
    union {
        lk_object_t  *lkobj;
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
LK_EXT_DEFINIT(lk_object_extinittypes);
LK_EXT_DEFINIT(lk_object_extinitfuncs);

/* new */
#define LK_OBJECT_MAXRECYCLED 1000
lk_object_t *lk_object_allocwithsize(lk_object_t *proto, size_t s);
lk_object_t *lk_object_alloc(lk_object_t *proto);
lk_object_t *lk_object_clone(lk_object_t *self);
void lk_object_justfree(lk_object_t *self);
void lk_object_free(lk_object_t *self);

/* update - tag */
#define LK_OBJECT_DEFTAGSETTER(t, field) \
void lk_object_set ## field(lk_object_t *self, t field)
LK_OBJECT_DEFTAGSETTER(lk_tagallocfunc_t *, allocfunc);
LK_OBJECT_DEFTAGSETTER(lk_tagmarkfunc_t *, markfunc);
LK_OBJECT_DEFTAGSETTER(lk_tagfreefunc_t *, freefunc);

/* update */
struct lk_slot *lk_object_setslot(lk_object_t *self, lk_object_t *k,
                                  lk_object_t *check, lk_object_t *v);
struct lk_slot *lk_object_setslotbycstr(lk_object_t *self, const char *k,
                                        lk_object_t *check, lk_object_t *v);
void lk_object_setvalueonslot(lk_object_t *self, struct lk_slot *slot,
                              lk_object_t *v);
int lk_object_calcancestors(lk_object_t *self);

/* info */
int lk_object_isa(lk_object_t *self, lk_object_t *t);
struct lk_slot *lk_object_getslot(lk_object_t *self, lk_object_t *k);
struct lk_slot *lk_object_getslotfromany(lk_object_t *self, lk_object_t *k);
lk_object_t *lk_object_getvaluefromslot(lk_object_t *self,
                                        struct lk_slot *slot);
int lk_object_hashcode(const void *k, int capa);
int lk_object_keycmp(const void *self, const void *other);
#define LK_OBJECT_ISTYPE(self, t) \
(  (self) == (t) \
|| (t) == LK_VM(self)->t_object \
/* || (  (t)->co.tag != LK_VM(self)->t_object->co.tag \
   && (self)->co.tag == (t)->co.tag \
   ) */ \
|| lk_object_isa((self), (t)) \
)
/*
#define LK_OBJECT_ISA(self, t) ((self) == (t) ? 1 : \
LK_OBJECT_HASONEPARENT((self)->co.parents) && \
LK_OBJECT_ONEPARENT((self)->co.parents) == (t) ? 2 : \
lk_object_isa((self), (t)))
#define LK_OBJECT_HASONEPARENT(pars) ((ptrdiff_t)(pars) & 1)
#define LK_OBJECT_ONEPARENT(pars) LK_O((ptrdiff_t)(pars) & ~1)
#define LK_OBJECT_PROTO(self) LK_OBJECT_HASONEPARENT((self)->co.parents) \
? LK_OBJECT_ONEPARENT((self)->co.parents) : LIST_ATPTR((self)->co.parents, 0)
 */
#define LK_OBJECT_HASPARENTS(self) ((ptrdiff_t)((self)->co.proto) & 1)
#define LK_OBJECT_PARENTS(self) ((list_t *)((ptrdiff_t)((self)->co.proto) & ~1))
#define LK_OBJECT_PROTO(self) ( \
    LK_OBJECT_HASPARENTS(self) \
    ? list_getptr(LK_OBJECT_PARENTS(self), -1) \
    : (self)->co.proto \
)
#define LK_OBJECT_ISA(self, t) ( \
    (self) == (t) ? 1 : \
    !LK_OBJECT_HASPARENTS(self) && (self)->co.proto == (t) ? 2 : \
    lk_object_isa((self), (t)) \
)
#define LK_OBJECT_ISCFUNC(self) ( \
    (self)->co.tag->allocfunc == LK_VM(self)->t_cfunc->co.tag->allocfunc)
#define LK_OBJECT_ISFRAME(self) ( \
    (self)->co.tag->freefunc == LK_VM(self)->t_frame->co.tag->freefunc)
#define LK_OBJECT_ISGFUNC(self) ( \
    (self)->co.tag->freefunc == LK_VM(self)->t_gfunc->co.tag->freefunc)
#define LK_OBJECT_ISFUNC(self) ( \
    (self)->co.tag->freefunc == LK_VM(self)->t_func->co.tag->freefunc || \
    LK_OBJECT_ISGFUNC(self) \
)
#define LK_OBJECT_ISINSTR(self) ( \
    (self)->co.tag->markfunc == LK_VM(self)->t_instr->co.tag->markfunc)
#endif
