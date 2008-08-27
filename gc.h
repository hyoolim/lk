#ifndef LK_GC_H
#define LK_GC_H
#include "types.h"

/* type */
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
void lk_gc_typeinit(lk_vm_t *vm);
void lk_gc_libinit(lk_vm_t *vm);

/* update */
void lk_objGroup_freeAll(struct lk_objGroup *self);
void lk_objGroup_remove(lk_obj_t *v);
void lk_objGroup_insert(struct lk_objGroup *self, lk_obj_t *v);
void lk_obj_markPending(lk_obj_t *self);
void lk_obj_markUsed(lk_obj_t *self);
lk_obj_t *lk_obj_addref(lk_obj_t *self, lk_obj_t *v);
void lk_gc_pause(lk_gc_t *self);
void lk_gc_resume(lk_gc_t *self);
void lk_gc_mark(lk_gc_t *self);
void lk_gc_sweep(lk_gc_t *self);

/* info */
int lk_objGroup_size(struct lk_objGroup *self);
struct lk_objGroup *lk_obj_objgroup(lk_obj_t *self);
#define LK_GC_ISMARKUNUSED(self, v) ( \
(v)->o.mark.objgroup == (self)->unused || (v)->o.mark.objgroup == NULL)
#define LK_GC_ISMARKPENDING(self, v) ( \
(v)->o.mark.objgroup == (self)->pending)
#define LK_GC_ISMARKUSED(self, v) ( \
(v)->o.mark.objgroup == (self)->used)
#endif
