#ifndef LK_GC_H
#define LK_GC_H

/* type */
typedef struct lk_Gc lk_Gc_t;
#include "vm.h"
#include "frame.h"
struct lk_Gc {
    struct lk_Common    obj;
    struct lk_Objectgroup *unused;
    struct lk_Objectgroup *pending;
    struct lk_Objectgroup *used;
    struct lk_Objectgroup *permanent;
    int                 newvalues;
    uint8_t             isrunning;
};
#define LK_GC(v) ((lk_Gc_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Gc_extinittypes);
LK_EXT_DEFINIT(lk_Gc_extinitfuncs);

/* update */
void lk_Objectgroup_freevalues(struct lk_Objectgroup *self);
void lk_Objectgroup_remove(lk_Object_t *v);
void lk_Objectgroup_insert(struct lk_Objectgroup *self, lk_Object_t *v);
void lk_Object_markpending(lk_Object_t *self);
void lk_Object_markused(lk_Object_t *self);
lk_Object_t *lk_Object_addref(lk_Object_t *self, lk_Object_t *v);
void lk_Gc_pause(lk_Gc_t *self);
void lk_Gc_resume(lk_Gc_t *self);
void lk_Gc_mark(lk_Gc_t *self);
void lk_Gc_sweep(lk_Gc_t *self);

/* info */
int lk_Objectgroup_count(struct lk_Objectgroup *self);
struct lk_Objectgroup *lk_Object_objgroup(lk_Object_t *self);
#define LK_GC_ISMARKUNUSED(self, v) ( \
(v)->obj.mark.objgroup == (self)->unused || (v)->obj.mark.objgroup == NULL)
#define LK_GC_ISMARKPENDING(self, v) ( \
(v)->obj.mark.objgroup == (self)->pending)
#define LK_GC_ISMARKUSED(self, o) ( \
(v)->obj.mark.objgroup == (self)->used)
#endif
