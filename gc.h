#ifndef LK_GC_H
#define LK_GC_H

/* type */
typedef struct lk_gc lk_gc_t;
#define LK_GC(v) ((lk_gc_t *)(v))
#include "vm.h"
#include "scope.h"
struct lk_gc {
    struct lk_common o;
    struct lk_objectgroup *unused;
    struct lk_objectgroup *pending;
    struct lk_objectgroup *used;
    struct lk_objectgroup *permanent;
    int                 newvalues;
    uint8_t             isrunning;
};

/* ext map */
LK_LIB_DEFINEINIT(lk_gc_libPreInit);
LK_LIB_DEFINEINIT(lk_gc_libInit);

/* update */
void lk_objectgroup_freevalues(struct lk_objectgroup *self);
void lk_objectgroup_remove(lk_object_t *v);
void lk_objectgroup_insert(struct lk_objectgroup *self, lk_object_t *v);
void lk_object_markpending(lk_object_t *self);
void lk_object_markused(lk_object_t *self);
lk_object_t *lk_object_addref(lk_object_t *self, lk_object_t *v);
void lk_gc_pause(lk_gc_t *self);
void lk_gc_resume(lk_gc_t *self);
void lk_gc_mark(lk_gc_t *self);
void lk_gc_sweep(lk_gc_t *self);

/* info */
int lk_objectgroup_size(struct lk_objectgroup *self);
struct lk_objectgroup *lk_object_objgroup(lk_object_t *self);
#define LK_GC_ISMARKUNUSED(self, v) ( \
(v)->o.mark.objgroup == (self)->unused || (v)->o.mark.objgroup == NULL)
#define LK_GC_ISMARKPENDING(self, v) ( \
(v)->o.mark.objgroup == (self)->pending)
#define LK_GC_ISMARKUSED(self, v) ( \
(v)->o.mark.objgroup == (self)->used)
#endif
