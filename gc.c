#include "gc.h"
#include "ext.h"

/* ext map - types */
static void free_gc(lk_object_t *self) {
    memory_free(LK_GC(self)->unused);
    memory_free(LK_GC(self)->pending);
    memory_free(LK_GC(self)->used);
    memory_free(LK_GC(self)->permanent);
}
void lk_gc_typeinit(lk_vm_t *vm) {
    vm->gc = LK_GC(lk_object_allocWithSize(vm->t_object, sizeof(lk_gc_t)));
    vm->gc->unused = memory_alloc(sizeof(struct lk_objGroup));
    vm->gc->pending = memory_alloc(sizeof(struct lk_objGroup));
    vm->gc->used = memory_alloc(sizeof(struct lk_objGroup));
    vm->gc->permanent = memory_alloc(sizeof(struct lk_objGroup));
    lk_object_setfreefunc(LK_OBJ(vm->gc), free_gc);
}

/* ext map - funcs */
static void pause_gc(lk_object_t *self, lk_scope_t *local) {
    lk_gc_pause(LK_GC(self)); RETURN(self); }
static void resume_gc(lk_object_t *self, lk_scope_t *local) {
    lk_gc_resume(LK_GC(self)); RETURN(self); }
void lk_gc_libinit(lk_vm_t *vm) {
    lk_object_t *gc = LK_OBJ(vm->gc);
    lk_lib_setGlobal("GarbageCollector", gc);
    lk_object_set_cfunc_lk(gc, "pause", pause_gc, NULL);
    lk_object_set_cfunc_lk(gc, "resume", resume_gc, NULL);
}

/* update */
void lk_objGroup_freeAll(struct lk_objGroup *self) {
    lk_object_t *c = self->first, *n;
    for(; c != NULL; c = n) {
        n = c->o.mark.next;
        lk_object_justfree(c);
    }
}
void lk_objGroup_remove(lk_object_t *v) {
    struct lk_objGroup *from = v->o.mark.objgroup;
    lk_object_t *p = v->o.mark.prev, *n = v->o.mark.next;
    if(p != NULL) p->o.mark.next = n;
    if(n != NULL) n->o.mark.prev = p;
    if(from != NULL) {
        if(from->first == v) from->first = n;
        if(from->last == v) from->last = p;
    }
}
void lk_objGroup_insert(struct lk_objGroup *self, lk_object_t *v) {
    v->o.mark.prev = self->last;
    v->o.mark.next = NULL;
    v->o.mark.objgroup = self;
    if(self->first == NULL) self->first = v;
    else (self->last->o.mark.next = v)->o.mark.prev = self->last;
    self->last = v;
}
void lk_object_markPending(lk_object_t *self) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKUNUSED(gc, self)) {
        lk_objGroup_remove(self);
        lk_objGroup_insert(gc->pending, self);
    }
}
static void gc_markpendingifunused(lk_object_t *v) {
    if(v != NULL) lk_object_markPending(v);
}
void lk_object_markUsed(lk_object_t *self) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKPENDING(gc, self)) {
        lk_objGroup_remove(self);
        lk_objGroup_insert(gc->used, self);
        if(LK_OBJ_HASPARENTS(self)) {
            LIST_EACHPTR(LK_OBJ_PARENTS(self), i, v,
                lk_object_markPending(LK_OBJ(v));
            );
        } else {
            if(self->o.parent != NULL) lk_object_markPending(self->o.parent);
        }
        if(self->o.slots != NULL) {
            struct lk_slot *slot;
            SET_EACH(self->o.slots, item,
                lk_object_markPending(LK_OBJ(item->key));
                slot = LK_SLOT(SETITEM_VALUEPTR(item));
                lk_object_markPending(slot->check);
                lk_object_markPending(lk_object_getvaluefromslot(self, slot));
            );
        }
        if(self->o.tag->markfunc != NULL) {
            self->o.tag->markfunc(self, gc_markpendingifunused);
        }
    }
}
lk_object_t *lk_object_addref(lk_object_t *self, lk_object_t *v) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKUSED(gc, self)) lk_object_markPending(v);
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
        lk_objGroup_size(self->unused),
        lk_objGroup_size(self->pending),
        lk_objGroup_size(self->used));
         */
        for(i = 0; i < 30000; i ++) {
            if(self->pending->first == NULL) { lk_gc_sweep(self); break; }
            lk_object_markUsed(self->pending->first);
        }
    }
}
void lk_gc_sweep(lk_gc_t *self) {
    if(self->isrunning) {
        lk_vm_t *vm = LK_VM(self);
        struct lk_objGroup *unused = self->unused;
        struct lk_rsrcchain *rsrc = vm->rsrc;
        /*
        printf("sweeping! - unused: %i, used: %i\n",
        lk_objGroup_size(self->unused),
        lk_objGroup_size(self->used));
         */
        lk_object_markPending(LK_OBJ(vm->currentScope));
        for(; rsrc != NULL; rsrc = rsrc->prev) {
            lk_object_markPending(LK_OBJ(rsrc->rsrc));
        }
        while(self->pending->first != NULL) {
            lk_object_markUsed(self->pending->first);
        }
        lk_objGroup_freeAll(unused);
        unused->first = unused->last = NULL;
        self->unused = self->used;
        self->used = unused;
    }
}

/* info */
int lk_objGroup_size(struct lk_objGroup *self) {
    int c = 0;
    lk_object_t *i;
    for(i = self->first; i != NULL; i = i->o.mark.next) c ++;
    return c;
}
