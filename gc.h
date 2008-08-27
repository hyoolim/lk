#ifndef LK_GC_H
#define LK_GC_H
#include "types.h"

/* type */
struct lk_gc {
    struct lk_common o;
    struct lk_objgroup *unused;
    struct lk_objgroup *pending;
    struct lk_objgroup *used;
    struct lk_objgroup *permanent;
    int                 newvalues;
    uint8_t             isrunning;
};

/* init */
void lk_gc_typeinit(lk_vm_t *vm);
void lk_gc_libinit(lk_vm_t *vm);

/* update */
void lk_gc_free_objgroup(struct lk_objgroup *self);
void lk_objgroup_remove(lk_obj_t *v);
void lk_objgroup_insert(struct lk_objgroup *self, lk_obj_t *v);
void lk_gc_mark_objpending(lk_obj_t *self);
void lk_gc_mark_objused(lk_obj_t *self);
lk_obj_t *lk_obj_addref(lk_obj_t *self, lk_obj_t *v);
void lk_gc_pause(lk_gc_t *self);
void lk_gc_resume(lk_gc_t *self);
void lk_gc_mark(lk_gc_t *self);
void lk_gc_sweep(lk_gc_t *self);

/* info */
int lk_objgroup_size(struct lk_objgroup *self);
struct lk_objgroup *lk_obj_objgroup(lk_obj_t *self);
#define LK_GC_ISMARKUNUSED(self, v) ( \
(v)->o.mark.objgroup == (self)->unused || (v)->o.mark.objgroup == NULL)
#define LK_GC_ISMARKPENDING(self, v) ( \
(v)->o.mark.objgroup == (self)->pending)
#define LK_GC_ISMARKUSED(self, v) ( \
(v)->o.mark.objgroup == (self)->used)
#endif
