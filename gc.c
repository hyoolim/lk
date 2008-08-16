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
LK_EXT_DEFINIT(lk_Gc_extinittypes) {
    vm->gc = LK_GC(lk_Object_allocwithsize(vm->t_obj, sizeof(lk_Gc_t)));
    vm->gc->unused = memory_alloc(sizeof(struct lk_Objectgroup));
    vm->gc->pending = memory_alloc(sizeof(struct lk_Objectgroup));
    vm->gc->used = memory_alloc(sizeof(struct lk_Objectgroup));
    vm->gc->permanent = memory_alloc(sizeof(struct lk_Objectgroup));
    lk_Object_setfreefunc(LK_OBJ(vm->gc), free__gc);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(pause__gc) {
    lk_Gc_pause(LK_GC(self)); RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(resume__gc) {
    lk_Gc_resume(LK_GC(self)); RETURN(self); }
LK_EXT_DEFINIT(lk_Gc_extinitfuncs) {
    lk_Object_t *gc = LK_OBJ(vm->gc);
    lk_Library_set(vm->t_vm, "GarbageCollector", gc);
    lk_Library_setCFunction(gc, "pause", pause__gc, NULL);
    lk_Library_setCFunction(gc, "resume", resume__gc, NULL);
}

/* update */
void lk_Objectgroup_freevalues(struct lk_Objectgroup *self) {
    lk_Object_t *c = self->first, *n;
    for(; c != NULL; c = n) {
        n = c->obj.mark.next;
        lk_Object_justfree(c);
    }
}
void lk_Objectgroup_remove(lk_Object_t *v) {
    struct lk_Objectgroup *from = v->obj.mark.objgroup;
    lk_Object_t *p = v->obj.mark.prev, *n = v->obj.mark.next;
    if(p != NULL) p->obj.mark.next = n;
    if(n != NULL) n->obj.mark.prev = p;
    if(from != NULL) {
        if(from->first == v) from->first = n;
        if(from->last == v) from->last = p;
    }
}
void lk_Objectgroup_insert(struct lk_Objectgroup *self, lk_Object_t *v) {
    v->obj.mark.prev = self->last;
    v->obj.mark.next = NULL;
    v->obj.mark.objgroup = self;
    if(self->first == NULL) self->first = v;
    else (self->last->obj.mark.next = v)->obj.mark.prev = self->last;
    self->last = v;
}
void lk_Object_markpending(lk_Object_t *self) {
    lk_Gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKUNUSED(gc, self)) {
        lk_Objectgroup_remove(self);
        lk_Objectgroup_insert(gc->pending, self);
    }
}
static void gc_markpendingifunused(lk_Object_t *v) {
    if(v != NULL) lk_Object_markpending(v);
}
void lk_Object_markused(lk_Object_t *self) {
    lk_Gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKPENDING(gc, self)) {
        lk_Objectgroup_remove(self);
        lk_Objectgroup_insert(gc->used, self);
        if(LK_OBJ_HASPARENTS(self)) {
            LIST_EACHPTR(LK_OBJ_PARENTS(self), i, v,
                lk_Object_markpending(LK_OBJ(v));
            );
        } else {
            if(self->obj.parent != NULL) lk_Object_markpending(self->obj.parent);
        }
        if(self->obj.slots != NULL) {
            struct lk_Slot *slot;
            SET_EACH(self->obj.slots, item,
                lk_Object_markpending(LK_OBJ(item->key));
                slot = LK_SLOT(SETITEM_VALUEPTR(item));
                lk_Object_markpending(slot->check);
                lk_Object_markpending(lk_Object_getvaluefromslot(self, slot));
            );
        }
        if(self->obj.tag->markfunc != NULL) {
            self->obj.tag->markfunc(self, gc_markpendingifunused);
        }
    }
}
lk_Object_t *lk_Object_addref(lk_Object_t *self, lk_Object_t *v) {
    lk_Gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKUSED(gc, self)) lk_Object_markpending(v);
    v->obj.mark.isref = 1;
    return v;
}
void lk_Gc_pause(lk_Gc_t *self) {
    self->isrunning = 0;
}
void lk_Gc_resume(lk_Gc_t *self) {
    self->isrunning = 1;
}
void lk_Gc_mark(lk_Gc_t *self) {
    if(self->isrunning) {
        int i;
        /*
        printf("marking! - unused: %i, pending: %i, used: %i\n",
        lk_Objectgroup_count(self->unused),
        lk_Objectgroup_count(self->pending),
        lk_Objectgroup_count(self->used));
         */
        for(i = 0; i < 30000; i ++) {
            if(self->pending->first == NULL) { lk_Gc_sweep(self); break; }
            lk_Object_markused(self->pending->first);
        }
    }
}
void lk_Gc_sweep(lk_Gc_t *self) {
    if(self->isrunning) {
        lk_Vm_t *vm = LK_VM(self);
        struct lk_Objectgroup *unused = self->unused;
        struct lk_Rsrcchain *rsrc = vm->rsrc;
        /*
        printf("sweeping! - unused: %i, used: %i\n",
        lk_Objectgroup_count(self->unused),
        lk_Objectgroup_count(self->used));
         */
        lk_Object_markpending(LK_OBJ(vm->currentFrame));
        for(; rsrc != NULL; rsrc = rsrc->prev) {
            lk_Object_markpending(LK_OBJ(rsrc->rsrc));
        }
        while(self->pending->first != NULL) {
            lk_Object_markused(self->pending->first);
        }
        lk_Objectgroup_freevalues(unused);
        unused->first = unused->last = NULL;
        self->unused = self->used;
        self->used = unused;
    }
}

/* info */
int lk_Objectgroup_count(struct lk_Objectgroup *self) {
    int c = 0;
    lk_Object_t *i;
    for(i = self->first; i != NULL; i = i->obj.mark.next) c ++;
    return c;
}
