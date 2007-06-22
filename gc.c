#include "gc.h"
#include "ext.h"
#define GC (LK_GC(self))

/* ext map - types */
static LK_OBJECT_DEFFREEFUNC(free__gc) {
    memory_free(GC->unused);
    memory_free(GC->pending);
    memory_free(GC->used);
    memory_free(GC->permanent);
}
LK_EXT_DEFINIT(lk_gc_extinittypes) {
    vm->gc = LK_GC(lk_object_allocwithsize(vm->t_object, sizeof(lk_gc_t)));
    vm->gc->unused = memory_alloc(sizeof(struct lk_objgroup));
    vm->gc->pending = memory_alloc(sizeof(struct lk_objgroup));
    vm->gc->used = memory_alloc(sizeof(struct lk_objgroup));
    vm->gc->permanent = memory_alloc(sizeof(struct lk_objgroup));
    lk_object_setfreefunc(LK_O(vm->gc), free__gc);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(pause__gc) {
    lk_gc_pause(LK_GC(self)); RETURN(self); }
static LK_EXT_DEFCFUNC(resume__gc) {
    lk_gc_resume(LK_GC(self)); RETURN(self); }
LK_EXT_DEFINIT(lk_gc_extinitfuncs) {
    lk_object_t *gc = LK_O(vm->gc);
    lk_ext_set(vm->t_vm, "GarbageCollector", gc);
    lk_ext_cfunc(gc, "pause", pause__gc, NULL);
    lk_ext_cfunc(gc, "resume", resume__gc, NULL);
}

/* update */
void lk_objgroup_freevalues(struct lk_objgroup *self) {
    lk_object_t *c = self->first, *n;
    for(; c != NULL; c = n) {
        n = c->co.mark.next;
        lk_object_justfree(c);
    }
}
void lk_objgroup_remove(lk_object_t *v) {
    struct lk_objgroup *from = v->co.mark.objgroup;
    lk_object_t *p = v->co.mark.prev, *n = v->co.mark.next;
    if(p != NULL) p->co.mark.next = n;
    if(n != NULL) n->co.mark.prev = p;
    if(from != NULL) {
        if(from->first == v) from->first = n;
        if(from->last == v) from->last = p;
    }
}
void lk_objgroup_insert(struct lk_objgroup *self, lk_object_t *v) {
    v->co.mark.prev = self->last;
    v->co.mark.next = NULL;
    v->co.mark.objgroup = self;
    if(self->first == NULL) self->first = v;
    else (self->last->co.mark.next = v)->co.mark.prev = self->last;
    self->last = v;
}
void lk_object_markpending(lk_object_t *self) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKUNUSED(gc, self)) {
        lk_objgroup_remove(self);
        lk_objgroup_insert(gc->pending, self);
    }
}
static void gc_markpendingifunused(lk_object_t *v) {
    if(v != NULL) lk_object_markpending(v);
}
void lk_object_markused(lk_object_t *self) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKPENDING(gc, self)) {
        lk_objgroup_remove(self);
        lk_objgroup_insert(gc->used, self);
        if(LK_OBJECT_HASPARENTS(self)) {
            LIST_EACHPTR(LK_OBJECT_PARENTS(self), i, v,
                lk_object_markpending(LK_O(v));
            );
        } else {
            if(self->co.proto != NULL) lk_object_markpending(self->co.proto);
        }
        if(self->co.slots != NULL) {
            struct lk_slotv *sv;
            SET_EACH(self->co.slots, item,
                lk_object_markpending(LK_O(item->key));
                sv = LK_SLOTV(SETITEM_VALUEPTR(item));
                lk_object_markpending(sv->type);
                lk_object_markpending(lk_object_getslotv(self, sv));
            );
        }
        if(self->co.tag->markfunc != NULL) {
            self->co.tag->markfunc(self, gc_markpendingifunused);
        }
    }
}
lk_object_t *lk_object_addref(lk_object_t *self, lk_object_t *v) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKUSED(gc, self)) lk_object_markpending(v);
    v->co.mark.isref = 1;
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
        lk_objgroup_count(self->unused),
        lk_objgroup_count(self->pending),
        lk_objgroup_count(self->used));
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
        struct lk_objgroup *unused = self->unused;
        struct lk_rsrcchain *rsrc = vm->rsrc;
        /*
        printf("sweeping! - unused: %i, used: %i\n",
        lk_objgroup_count(self->unused),
        lk_objgroup_count(self->used));
         */
        lk_object_markpending(LK_O(vm->currframe));
        LIST_EACHPTR(vm->retained, i, v, lk_object_markpending(LK_O(v)));
        SET_EACH(vm->symbols, i, lk_object_markpending(LK_O(i->key)));
        for(; rsrc != NULL; rsrc = rsrc->prev) {
            lk_object_markpending(LK_O(rsrc->rsrc));
        }
        while(self->pending->first != NULL) {
            lk_object_markused(self->pending->first);
        }
        lk_objgroup_freevalues(unused);
        unused->first = unused->last = NULL;
        self->unused = self->used;
        self->used = unused;
    }
}

/* info */
int lk_objgroup_count(struct lk_objgroup *self) {
    int c = 0;
    lk_object_t *i;
    for(i = self->first; i != NULL; i = i->co.mark.next) c ++;
    return c;
}
