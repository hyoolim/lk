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
    vm->gc = LK_GC(lk_obj_allocwithsize(vm->t_obj, sizeof(lk_gc_t)));
    vm->gc->unused = memory_alloc(sizeof(struct lk_objgroup));
    vm->gc->pending = memory_alloc(sizeof(struct lk_objgroup));
    vm->gc->used = memory_alloc(sizeof(struct lk_objgroup));
    vm->gc->permanent = memory_alloc(sizeof(struct lk_objgroup));
    lk_obj_setfreefunc(LK_OBJ(vm->gc), free__gc);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(pause__gc) {
    lk_gc_pause(LK_GC(self)); RETURN(self); }
static LK_EXT_DEFCFUNC(resume__gc) {
    lk_gc_resume(LK_GC(self)); RETURN(self); }
LK_EXT_DEFINIT(lk_gc_extinitfuncs) {
    lk_obj_t *gc = LK_OBJ(vm->gc);
    lk_ext_set(vm->t_vm, "GarbageCollector", gc);
    lk_ext_cfunc(gc, "pause", pause__gc, NULL);
    lk_ext_cfunc(gc, "resume", resume__gc, NULL);
}

/* update */
void lk_objgroup_freevalues(struct lk_objgroup *self) {
    lk_obj_t *c = self->first, *n;
    for(; c != NULL; c = n) {
        n = c->obj.mark.next;
        lk_obj_justfree(c);
    }
}
void lk_objgroup_remove(lk_obj_t *v) {
    struct lk_objgroup *from = v->obj.mark.objgroup;
    lk_obj_t *p = v->obj.mark.prev, *n = v->obj.mark.next;
    if(p != NULL) p->obj.mark.next = n;
    if(n != NULL) n->obj.mark.prev = p;
    if(from != NULL) {
        if(from->first == v) from->first = n;
        if(from->last == v) from->last = p;
    }
}
void lk_objgroup_insert(struct lk_objgroup *self, lk_obj_t *v) {
    v->obj.mark.prev = self->last;
    v->obj.mark.next = NULL;
    v->obj.mark.objgroup = self;
    if(self->first == NULL) self->first = v;
    else (self->last->obj.mark.next = v)->obj.mark.prev = self->last;
    self->last = v;
}
void lk_obj_markpending(lk_obj_t *self) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKUNUSED(gc, self)) {
        lk_objgroup_remove(self);
        lk_objgroup_insert(gc->pending, self);
    }
}
static void gc_markpendingifunused(lk_obj_t *v) {
    if(v != NULL) lk_obj_markpending(v);
}
void lk_obj_markused(lk_obj_t *self) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKPENDING(gc, self)) {
        lk_objgroup_remove(self);
        lk_objgroup_insert(gc->used, self);
        if(LK_OBJ_HASPARENTS(self)) {
            LIST_EACHPTR(LK_OBJ_PARENTS(self), i, v,
                lk_obj_markpending(LK_OBJ(v));
            );
        } else {
            if(self->obj.proto != NULL) lk_obj_markpending(self->obj.proto);
        }
        if(self->obj.slots != NULL) {
            struct lk_slot *slot;
            SET_EACH(self->obj.slots, item,
                lk_obj_markpending(LK_OBJ(item->key));
                slot = LK_SLOT(SETITEM_VALUEPTR(item));
                lk_obj_markpending(slot->check);
                lk_obj_markpending(lk_obj_getvaluefromslot(self, slot));
            );
        }
        if(self->obj.tag->markfunc != NULL) {
            self->obj.tag->markfunc(self, gc_markpendingifunused);
        }
    }
}
lk_obj_t *lk_obj_addref(lk_obj_t *self, lk_obj_t *v) {
    lk_gc_t *gc = LK_VM(self)->gc;
    if(LK_GC_ISMARKUSED(gc, self)) lk_obj_markpending(v);
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
        lk_objgroup_count(self->unused),
        lk_objgroup_count(self->pending),
        lk_objgroup_count(self->used));
         */
        for(i = 0; i < 30000; i ++) {
            if(self->pending->first == NULL) { lk_gc_sweep(self); break; }
            lk_obj_markused(self->pending->first);
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
        lk_obj_markpending(LK_OBJ(vm->currframe));
        LIST_EACHPTR(vm->retained, i, v, lk_obj_markpending(LK_OBJ(v)));
        SET_EACH(vm->symbols, i, lk_obj_markpending(LK_OBJ(i->key)));
        for(; rsrc != NULL; rsrc = rsrc->prev) {
            lk_obj_markpending(LK_OBJ(rsrc->rsrc));
        }
        while(self->pending->first != NULL) {
            lk_obj_markused(self->pending->first);
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
    lk_obj_t *i;
    for(i = self->first; i != NULL; i = i->obj.mark.next) c ++;
    return c;
}
