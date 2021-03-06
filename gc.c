#include "lib.h"

/* ext map - types */
static void free_gc(lk_obj_t *self) {
    mem_free(LK_GC(self)->unused);
    mem_free(LK_GC(self)->pending);
    mem_free(LK_GC(self)->used);
    mem_free(LK_GC(self)->permanent);
}
void lk_gc_typeinit(lk_vm_t *vm) {
    vm->gc = LK_GC(lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_gc_t)));
    vm->gc->unused = mem_alloc(sizeof(struct lk_objgroup));
    vm->gc->pending = mem_alloc(sizeof(struct lk_objgroup));
    vm->gc->used = mem_alloc(sizeof(struct lk_objgroup));
    vm->gc->permanent = mem_alloc(sizeof(struct lk_objgroup));
    lk_obj_setfreefunc(LK_OBJ(vm->gc), free_gc);
}

/* update */
void lk_gc_free_objgroup(struct lk_objgroup *self) {
    lk_obj_t *c = self->first, *n;
    for(; c != NULL; c = n) {
        n = c->o.mark.next;
        lk_obj_justfree(c);
    }
}
void lk_objgroup_remove(lk_obj_t *v) {
    struct lk_objgroup *from = v->o.mark.objgroup;
    lk_obj_t *p = v->o.mark.prev, *n = v->o.mark.next;
    if(p != NULL) p->o.mark.next = n;
    if(n != NULL) n->o.mark.prev = p;
    if(from != NULL) {
        if(from->first == v) from->first = n;
        if(from->last == v) from->last = p;
    }
}
void lk_objgroup_insert(struct lk_objgroup *self, lk_obj_t *v) {
    v->o.mark.prev = self->last;
    v->o.mark.next = NULL;
    v->o.mark.objgroup = self;
    if(self->first == NULL) self->first = v;
    else (self->last->o.mark.next = v)->o.mark.prev = self->last;
    self->last = v;
}
void lk_gc_mark_objpending(lk_obj_t *self) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKUNUSED(gc, self)) {
        lk_objgroup_remove(self);
        lk_objgroup_insert(gc->pending, self);
    }
}
static void gc_markpendingifunused(lk_obj_t *v) {
    if(v != NULL) lk_gc_mark_objpending(v);
}
void lk_gc_mark_objused(lk_obj_t *self) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKPENDING(gc, self)) {
        lk_objgroup_remove(self);
        lk_objgroup_insert(gc->used, self);
        if(LK_OBJ_HASPARENTS(self)) {
            DARRAY_EACHPTR(LK_OBJ_PARENTS(self), i, v,
                lk_gc_mark_objpending(LK_OBJ(v));
            );
        } else {
            if(self->o.parent != NULL) lk_gc_mark_objpending(self->o.parent);
        }
        if(self->o.slots != NULL) {
            struct lk_slot *slot;
            SET_EACH(self->o.slots, item,
                lk_gc_mark_objpending(LK_OBJ(item->key));
                slot = LK_SLOT(SETITEM_VALUEPTR(item));
                lk_gc_mark_objpending(slot->check);
                lk_gc_mark_objpending(lk_obj_getvaluefromslot(self, slot));
            );
        }
        if(self->o.tag->markfunc != NULL) {
            self->o.tag->markfunc(self, gc_markpendingifunused);
        }
    }
}
lk_obj_t *lk_obj_addref(lk_obj_t *self, lk_obj_t *v) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKUSED(gc, self)) lk_gc_mark_objpending(v);
    v->o.mark.isref = 1;
    return v;
}
void lk_gc_pause(lk_gc_t *self) {
    self->isrunning = 0;
}
void lk_gc_resume(lk_gc_t *self) {
    self->isrunning = 1;
}
void lk_gc_mark(lk_gc_t *self) {
    if(self->isrunning) {
        int i;
        /*
        printf("marking! - unused: %i, pending: %i, used: %i\n",
        lk_objgroup_size(self->unused),
        lk_objgroup_size(self->pending),
        lk_objgroup_size(self->used));
         */
        for(i = 0; i < 30000; i ++) {
            if(self->pending->first == NULL) { lk_gc_sweep(self); break; }
            lk_gc_mark_objused(self->pending->first);
        }
    }
}
void lk_gc_sweep(lk_gc_t *self) {
    if(self->isrunning) {
        lk_vm_t *vm = LK_VM(self);
        struct lk_objgroup *unused = self->unused;
        struct lk_rsrcchain *rsrc = vm->rsrc;
        /*
        printf("sweeping! - unused: %i, used: %i\n",
        lk_objgroup_size(self->unused),
        lk_objgroup_size(self->used));
         */
        lk_gc_mark_objpending(LK_OBJ(vm->currscope));
        for(; rsrc != NULL; rsrc = rsrc->prev) {
            lk_gc_mark_objpending(LK_OBJ(rsrc->rsrc));
        }
        while(self->pending->first != NULL) {
            lk_gc_mark_objused(self->pending->first);
        }
        lk_gc_free_objgroup(unused);
        unused->first = unused->last = NULL;
        self->unused = self->used;
        self->used = unused;
    }
}

/* info */
int lk_objgroup_size(struct lk_objgroup *self) {
    int c = 0;
    lk_obj_t *i;
    for(i = self->first; i != NULL; i = i->o.mark.next) c ++;
    return c;
}

/* bind all c funcs to lk equiv */
void lk_gc_libinit(lk_vm_t *vm) {
    lk_obj_t *gc = LK_OBJ(vm->gc);
    lk_global_set("GarbageCollector", gc);

    /* update */
    lk_obj_set_cfunc_cvoid(gc, "pause!", lk_gc_pause, NULL);
    lk_obj_set_cfunc_cvoid(gc, "resume!", lk_gc_resume, NULL);
}
