#include "gc.h"
#include "ext.h"
#define GC (LK_GC(self))

/* ext map - types */
static LK_OBJ_DEFFREEFUNC(free__gc) {
    memory_free(GC->unused);
    memory_free(GC->pending);
    memory_free(GC->used);
    memory_free(GC->permanent);
}
LK_EXT_DEFINIT(lk_gc_extinittypes) {
    vm->gc = LK_GC(lk_object_allocwithsize(vm->t_obj, sizeof(lk_gc_t)));
    vm->gc->unused = memory_alloc(sizeof(struct lk_objectgroup));
    vm->gc->pending = memory_alloc(sizeof(struct lk_objectgroup));
    vm->gc->used = memory_alloc(sizeof(struct lk_objectgroup));
    vm->gc->permanent = memory_alloc(sizeof(struct lk_objectgroup));
    lk_object_setfreefunc(LK_OBJ(vm->gc), free__gc);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(pause__gc) {
    lk_gc_pause(LK_GC(self)); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(resume__gc) {
    lk_gc_resume(LK_GC(self)); RETURN(self); }
LK_EXT_DEFINIT(lk_gc_extinitfuncs) {
    lk_object_t *gc = LK_OBJ(vm->gc);
    lk_library_set(vm->t_vm, "GarbageCollector", gc);
    lk_library_setCFunction(gc, "pause", pause__gc, NULL);
    lk_library_setCFunction(gc, "resume", resume__gc, NULL);
}

/* update */
void lk_objectgroup_freevalues(struct lk_objectgroup *self) {
    lk_object_t *c = self->first, *n;
    for(; c != NULL; c = n) {
        n = c->obj.mark.next;
        lk_object_justfree(c);
    }
}
void lk_objectgroup_remove(lk_object_t *v) {
    struct lk_objectgroup *from = v->obj.mark.objgroup;
    lk_object_t *p = v->obj.mark.prev, *n = v->obj.mark.next;
    if(p != NULL) p->obj.mark.next = n;
    if(n != NULL) n->obj.mark.prev = p;
    if(from != NULL) {
        if(from->first == v) from->first = n;
        if(from->last == v) from->last = p;
    }
}
void lk_objectgroup_insert(struct lk_objectgroup *self, lk_object_t *v) {
    v->obj.mark.prev = self->last;
    v->obj.mark.next = NULL;
    v->obj.mark.objgroup = self;
    if(self->first == NULL) self->first = v;
    else (self->last->obj.mark.next = v)->obj.mark.prev = self->last;
    self->last = v;
}
void lk_object_markpending(lk_object_t *self) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKUNUSED(gc, self)) {
        lk_objectgroup_remove(self);
        lk_objectgroup_insert(gc->pending, self);
    }
}
static void gc_markpendingifunused(lk_object_t *v) {
    if(v != NULL) lk_object_markpending(v);
}
void lk_object_markused(lk_object_t *self) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKPENDING(gc, self)) {
        lk_objectgroup_remove(self);
        lk_objectgroup_insert(gc->used, self);
        if(LK_OBJ_HASPARENTS(self)) {
            LIST_EACHPTR(LK_OBJ_PARENTS(self), i, v,
                lk_object_markpending(LK_OBJ(v));
            );
        } else {
            if(self->obj.parent != NULL) lk_object_markpending(self->obj.parent);
        }
        if(self->obj.slots != NULL) {
            struct lk_slot *slot;
            SET_EACH(self->obj.slots, item,
                lk_object_markpending(LK_OBJ(item->key));
                slot = LK_SLOT(SETITEM_VALUEPTR(item));
                lk_object_markpending(slot->check);
                lk_object_markpending(lk_object_getvaluefromslot(self, slot));
            );
        }
        if(self->obj.tag->markfunc != NULL) {
            self->obj.tag->markfunc(self, gc_markpendingifunused);
        }
    }
}
lk_object_t *lk_object_addref(lk_object_t *self, lk_object_t *v) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKUSED(gc, self)) lk_object_markpending(v);
    v->obj.mark.isref = 1;
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
        lk_objectgroup_size(self->unused),
        lk_objectgroup_size(self->pending),
        lk_objectgroup_size(self->used));
         */
        for(i = 0; i < 30000; i ++) {
            if(self->pending->first == NULL) { lk_gc_sweep(self); break; }
            lk_object_markused(self->pending->first);
        }
    }
}
void lk_gc_sweep(lk_gc_t *self) {
    if(self->isrunning) {
        lk_vm_t *vm = LK_VM(self);
        struct lk_objectgroup *unused = self->unused;
        struct lk_rsrcchain *rsrc = vm->rsrc;
        /*
        printf("sweeping! - unused: %i, used: %i\n",
        lk_objectgroup_size(self->unused),
        lk_objectgroup_size(self->used));
         */
        lk_object_markpending(LK_OBJ(vm->currentFrame));
        for(; rsrc != NULL; rsrc = rsrc->prev) {
            lk_object_markpending(LK_OBJ(rsrc->rsrc));
        }
        while(self->pending->first != NULL) {
            lk_object_markused(self->pending->first);
        }
        lk_objectgroup_freevalues(unused);
        unused->first = unused->last = NULL;
        self->unused = self->used;
        self->used = unused;
    }
}

/* info */
int lk_objectgroup_size(struct lk_objectgroup *self) {
    int c = 0;
    lk_object_t *i;
    for(i = self->first; i != NULL; i = i->obj.mark.next) c ++;
    return c;
}
