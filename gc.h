#ifndef LK_GC_H
#define LK_GC_H

/* type */
typedef struct lk_gc lk_gc_t;
#include "vm.h"
#include "frame.h"
struct lk_gc {
    struct lk_common    co;
    struct lk_objgroup *unused;
    struct lk_objgroup *pending;
    struct lk_objgroup *used;
    struct lk_objgroup *permanent;
    int                 newvalues;
    uint8_t             isrunning;
};
#define LK_GC(v) ((lk_gc_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_gc_extinittypes);
LK_EXT_DEFINIT(lk_gc_extinitfuncs);

/* update */
void lk_objgroup_freevalues(struct lk_objgroup *self);
void lk_objgroup_remove(lk_object_t *v);
void lk_objgroup_insert(struct lk_objgroup *self, lk_object_t *v);
void lk_object_markpending(lk_object_t *self);
void lk_object_markused(lk_object_t *self);
lk_object_t *lk_object_addref(lk_object_t *self, lk_object_t *v);
void lk_gc_pause(lk_gc_t *self);
void lk_gc_resume(lk_gc_t *self);
void lk_gc_mark(lk_gc_t *self);
void lk_gc_sweep(lk_gc_t *self);

/* info */
int lk_objgroup_count(struct lk_objgroup *self);
struct lk_objgroup *lk_object_objgroup(lk_object_t *self);
#define LK_GC_ISMARKUNUSED(self, v) ( \
(v)->co.mark.objgroup == (self)->unused || (v)->co.mark.objgroup == NULL)
#define LK_GC_ISMARKPENDING(self, v) ( \
(v)->co.mark.objgroup == (self)->pending)
#define LK_GC_ISMARKUSED(self, o) ( \
(v)->co.mark.objgroup == (self)->used)
#endif
