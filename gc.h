#ifndef LK_GC_H
#define LK_GC_H

/* type */
typedef struct lk_gc lk_gc_t;
#define LK_GC(self) ((lk_gc_t *)(self))
#include "vm.h"
#include "scope.h"
struct lk_gc {
    struct lk_common o;
    struct lk_objGroup *unused;
    struct lk_objGroup *pending;
    struct lk_objGroup *used;
    struct lk_objGroup *permanent;
    int                 newvalues;
    uint8_t             isrunning;
};

/* init */
LK_LIB_DEFINEINIT(lk_gc_libPreInit);
LK_LIB_DEFINEINIT(lk_gc_libInit);

/* update */
void lk_objGroup_freeAll(struct lk_objGroup *self);
void lk_objGroup_remove(lk_object_t *v);
void lk_objGroup_insert(struct lk_objGroup *self, lk_object_t *v);
void lk_object_markPending(lk_object_t *self);
void lk_object_markUsed(lk_object_t *self);
lk_object_t *lk_object_addref(lk_object_t *self, lk_object_t *v);
void lk_gc_pause(lk_gc_t *self);
void lk_gc_resume(lk_gc_t *self);
void lk_gc_mark(lk_gc_t *self);
void lk_gc_sweep(lk_gc_t *self);

/* info */
int lk_objGroup_size(struct lk_objGroup *self);
struct lk_objGroup *lk_object_objectgroup(lk_object_t *self);
#define LK_GC_ISMARKUNUSED(self, v) ( \
(v)->o.mark.objgroup == (self)->unused || (v)->o.mark.objgroup == NULL)
#define LK_GC_ISMARKPENDING(self, v) ( \
(v)->o.mark.objgroup == (self)->pending)
#define LK_GC_ISMARKUSED(self, v) ( \
(v)->o.mark.objgroup == (self)->used)
#endif
