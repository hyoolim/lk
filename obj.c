#include "obj.h"
#include "ext.h"
#include "fixnum.h"
#include "gc.h"
#include "list.h"
#include "map.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_object_extinittypes) {
    lk_object_t *o = vm->t_obj = memory_alloc(sizeof(lk_object_t));
    o->obj.tag = memory_alloc(sizeof(struct lk_tag));
    o->obj.tag->refc = 1;
    o->obj.tag->vm = vm;
    o->obj.tag->size = sizeof(lk_object_t);
    o->obj.parent = NULL;
    array_pushptr(o->obj.ancestors = array_allocptr(), o);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(Ddefine_and_assignB__obj_str_obj_obj);
LK_LIBRARY_DEFINECFUNCTION(Ddefine__obj_str_obj) {
    env->argc ++;
    array_pushptr(&env->stack, N);
    GOTO(Ddefine_and_assignB__obj_str_obj_obj);
}
LK_LIBRARY_DEFINECFUNCTION(Ddefine_and_assignB__obj_str_obj) {
    env->argc ++;
    array_insertptr(&env->stack, 1, VM->t_obj);
    GOTO(Ddefine_and_assignB__obj_str_obj_obj);
}
LK_LIBRARY_DEFINECFUNCTION(Ddefine_and_assignB__obj_str_obj_obj) {
    lk_object_t *k = ARG(0);
    lk_object_t *v = ARG(2);
    struct lk_slot *slot;
    if(self->obj.slots != NULL && set_get(self->obj.slots, k) != NULL) {
        lk_vm_raisecstr(VM, "Cannot redefine %s", k);
    }
    slot = lk_object_setslot(self, k, ARG(1), v);
    v = lk_object_getvaluefromslot(self, slot);
    if(LK_OBJ_ISFUNC(v)) {
        LK_SLOT_SETOPTION(slot, LK_SLOTOPTION_AUTOSEND);
        SETOPT(LK_FUNC(v)->cf.opts, LK_FUNCOASSIGNED);
        LK_FUNC(v)->cf.doc = env->caller->current->prev->comment;
    }
    RETURN(v);
}
LK_LIBRARY_DEFINECFUNCTION(Did__obj) {
    RETURN(lk_fi_new(VM, (int)self)); }
LK_LIBRARY_DEFINECFUNCTION(Dretrieve__obj_str) {
    struct lk_slot *slot = lk_object_getslotfromany(self, ARG(0));
    if(slot != NULL) RETURN(lk_object_getvaluefromslot(self, slot));
    else RETURN(N);

}
LK_LIBRARY_DEFINECFUNCTION(Dself__obj) {
    RETURN(self); }
LK_LIBRARY_DEFINECFUNCTION(Dslots__obj) {
    lk_list_t *slots = lk_list_new(VM);
    if(self->obj.slots != NULL) {
        SET_EACH(self->obj.slots, i,
            array_pushptr(LIST(slots), (void *)i->key);
        );
    }
    RETURN(slots);
}
LK_LIBRARY_DEFINECFUNCTION(alloc__obj) {
    RETURN(lk_object_alloc(self)); }
LK_LIBRARY_DEFINECFUNCTION(also__obj_obj) {
    array_t *pars;
    if(LK_OBJ_HASPARENTS(self)) {
        pars = LK_OBJ_PARENTS(self);
    } else {
        pars = array_allocptr();
        array_pushptr(pars, self->obj.parent);
        self->obj.parent = LK_OBJ((ptrdiff_t)pars | 1);
    }
    array_unshiftptr(pars, ARG(0));
    if(lk_object_calcancestors(self)) {
        RETURN(self);
    } else {
        printf("BUG: Throw proper ancestor error here\n");
        exit(EXIT_FAILURE);
    }
}
LK_LIBRARY_DEFINECFUNCTION(ancestor__obj_obj) {
    RETURN(LK_OBJ_ISTYPE(self, ARG(0)) ? VM->t_true : VM->t_false); }
LK_LIBRARY_DEFINECFUNCTION(ancestors__obj) {
    if(self->obj.ancestors != NULL || lk_object_calcancestors(self)) {
        RETURN(lk_list_newfromlist(VM, self->obj.ancestors));
    } else {
        printf("BUG: Throw proper ancestor error here\n");
        exit(EXIT_FAILURE);
    }
}
LK_LIBRARY_DEFINECFUNCTION(clone__obj) {
    RETURN(lk_object_clone(self)); }
LK_LIBRARY_DEFINECFUNCTION(do__obj_f) {
    lk_kfunc_t *kf = LK_KFUNC(ARG(0));
    lk_frame_t *fr = lk_frame_new(VM);
    fr->first = fr->next = kf->first;
    fr->receiver = fr->self = self;
    fr->func = LK_OBJ(kf);
    fr->returnto = NULL;
    fr->obj.parent = LK_OBJ(kf->frame);
    lk_vm_doevalfunc(VM);
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(import__obj_obj) {
    set_t *from = ARG(0)->obj.slots;
    if(from != NULL) {
        set_t *to = self->obj.slots;
        if(to == NULL) to = self->obj.slots = set_alloc(
        sizeof(struct lk_slot), lk_object_hashcode, lk_object_keycmp);
        SET_EACH(from, i,
            *LK_SLOT(set_set(to, i->key)) = *LK_SLOT(SETITEM_VALUEPTR(i));
        );
    }
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(parents__obj) {
    if(LK_OBJ_HASPARENTS(self)) {
        RETURN(lk_list_newfromlist(VM, LK_OBJ_PARENTS(self)));
    } else {
        lk_list_t *ret = lk_list_new(VM);
        array_pushptr(LIST(ret), self->obj.parent);
        RETURN(ret);
    }
}
LK_LIBRARY_DEFINECFUNCTION(parent__obj) {
    if(LK_OBJ_HASPARENTS(self)) {
        array_t *pars = LK_OBJ_PARENTS(self);
        RETURN(LIST_COUNT(pars) > 0 ? array_getptr(pars, -1) : N);
    }
    RETURN(self->obj.parent);
}
LK_LIBRARY_DEFINECFUNCTION(with__obj_f) {
    do__obj_f(lk_object_addref(LK_OBJ(env), lk_object_alloc(self)), env);
}
LK_EXT_DEFINIT(lk_object_extinitfuncs) {
    lk_object_t *obj = vm->t_obj, *str = vm->t_string, *f = vm->t_func;
    lk_library_setGlobal("Object", obj);
    lk_library_setCFunction(obj, ".", Dself__obj, NULL);
    lk_library_setCFunction(obj, ":", Ddefine__obj_str_obj, str, obj, NULL);
    lk_library_setCFunction(obj, ":=", Ddefine_and_assignB__obj_str_obj, str, obj, NULL);
    lk_library_setCFunction(obj, ":=", Ddefine_and_assignB__obj_str_obj_obj, str, obj, obj, NULL);
    lk_library_setCFunction(obj, ".id", Did__obj, NULL);
    lk_library_setCFunction(obj, ".retrieve", Dretrieve__obj_str, str, NULL);
    lk_library_setCFunction(obj, ".self", Dself__obj, NULL);
    lk_library_setCFunction(obj, ".slots", Dslots__obj, NULL);
    lk_library_setCFunction(obj, "alloc", alloc__obj, NULL);
    lk_library_setCFunction(obj, "also", also__obj_obj, obj, NULL);
    lk_library_setCFunction(obj, "ancestor?", ancestor__obj_obj, obj, NULL);
    lk_library_setCFunction(obj, "ancestors", ancestors__obj, NULL);
    lk_library_setCFunction(obj, "clone", clone__obj, NULL);
    lk_library_setCFunction(obj, "do", do__obj_f, f, NULL);
    lk_library_setCFunction(obj, "import", import__obj_obj, obj, NULL);
    lk_library_setCFunction(obj, "parents", parents__obj, NULL);
    lk_library_setCFunction(obj, "parent", parent__obj, NULL);
    lk_library_setCFunction(obj, "with", with__obj_f, f, NULL);
}

/* new */
static struct lk_tag *tag_clone(struct lk_tag *self) {
    struct lk_tag *clone = memory_alloc(sizeof(struct lk_tag));
    memcpy(clone, self, sizeof(struct lk_tag));
    clone->refc = 1;
    return clone;
}
lk_object_t *lk_object_allocwithsize(lk_object_t *parent, size_t s) {
    lk_gc_t *gc = LK_VM(parent)->gc;
    lk_object_t *self = memory_alloc(s);
    struct lk_tag *tag = parent->obj.tag;
    if(tag->size == s) tag->refc ++;
    else (tag = tag_clone(tag))->size = s;
    self->obj.parent = parent;
    self->obj.tag = tag;
    if(tag->allocfunc != NULL) tag->allocfunc(self, parent);
    if(gc != NULL) {
        gc->newvalues ++;
        lk_objectgroup_insert(gc->unused, self);
    }
    return self;
}
lk_object_t *lk_object_alloc(lk_object_t *parent) {
    return lk_object_allocwithsize(parent, parent->obj.tag->size);
}
lk_object_t *lk_object_clone(lk_object_t *self) {
    lk_object_t *c = lk_object_alloc(LK_OBJ_PROTO(self));
    if(c->obj.tag->allocfunc != NULL) c->obj.tag->allocfunc(c, self);
    return c;
}
void lk_object_justfree(lk_object_t *self) {
    struct lk_tag *tag = self->obj.tag;
    if(tag->freefunc != NULL) tag->freefunc(self);
    if(LK_OBJ_HASPARENTS(self)) array_free(LK_OBJ_PARENTS(self));
    if(self->obj.ancestors != NULL) array_free(self->obj.ancestors);
    if(self->obj.slots != NULL) set_free(self->obj.slots);
    if(-- tag->refc < 1) memory_free(tag);
    memory_free(self);
}
void lk_object_free(lk_object_t *self) {
    lk_objectgroup_remove(self);
    lk_object_justfree(self);
}

/* update - tag */
#define LK_OBJ_IMPLTAGSETTER(t, field) \
LK_OBJ_DEFTAGSETTER(t, field) { \
    struct lk_tag *tag = self->obj.tag; \
    if(tag->field != field) { \
        if(tag->refc > 1) { \
            tag->refc --; \
            tag = self->obj.tag = tag_clone(tag); \
        } \
        tag->field = field; \
    } \
} LK_OBJ_DEFTAGSETTER(t, field)
LK_OBJ_IMPLTAGSETTER(lk_tagallocfunc_t *, allocfunc);
LK_OBJ_IMPLTAGSETTER(lk_tagmarkfunc_t *, markfunc);
LK_OBJ_IMPLTAGSETTER(lk_tagfreefunc_t *, freefunc);

/* update */
struct lk_slot *lk_object_setslot(lk_object_t *self, lk_object_t *k,
                                  lk_object_t *check, lk_object_t *v) {
    struct lk_slot *slot = lk_object_getslot(self, k);
    if(slot == NULL) {
        uint32_t first = array_getuchar(LIST(k), 0);
        if(self->obj.slots == NULL) {
            self->obj.slots = set_alloc(sizeof(struct lk_slot),
                                       lk_object_hashcode,
                                       lk_object_keycmp);
        }
        slot = LK_SLOT(set_set(self->obj.slots, k));
        slot->check = check;
        if('A' <= first && first <= 'Z') {
            LK_SLOT_SETOPTION(slot, LK_SLOTOPTION_READONLY);
            lk_object_setslot(v, LK_OBJ(LK_VM(self)->str_type),
                              LK_OBJ(LK_VM(self)->t_string), LK_OBJ(k));
        }
    }
    lk_object_setvalueonslot(self, slot, v);
    return slot;
}
struct lk_slot *lk_object_setslotbycstr(lk_object_t *self, const char *k,
                                        lk_object_t *check, lk_object_t *v) {
    return lk_object_setslot(self,
    LK_OBJ(lk_string_newfromcstr(LK_VM(self), k)), check, v);
}
void lk_object_setvalueonslot(lk_object_t *self, struct lk_slot *slot,
                              lk_object_t *v) {
    lk_vm_t *vm = LK_VM(self);
    if(v == NULL) v = vm->t_nil;
    if(v == vm->t_nil || LK_OBJ_ISTYPE(v, slot->check)) {
        lk_object_addref(self, v);
        if(LK_SLOT_GETTYPE(slot) == LK_SLOTTYPE_CFIELDLKOBJ) {
            if(v == vm->t_nil) v = NULL;
            *(lk_object_t **)((ptrdiff_t)self + slot->value.coffset) = v;
        } else {
            slot->value.lkobj = v;
        }
    } else {
        printf("type mismatch!\n");
        exit(EXIT_FAILURE);
    }
}
int lk_object_calcancestors(lk_object_t *self) {
    lk_object_t *cand = NULL;
    array_t *pars;
    array_t *il, *newancs = array_allocptr(), *ol = array_allocptr();
    int candi, j, k, olc, ilc;
    il = array_allocptr();
    array_pushptr(il, self);
    array_pushptr(ol, il);
    if(LK_OBJ_HASPARENTS(self)) {
        pars = LK_OBJ_PARENTS(self);
        for(candi = 0; candi < LIST_COUNT(pars); candi ++) {
            cand = LIST_ATPTR(pars, candi);
            il = array_allocptr();
            if(cand->obj.ancestors == NULL) lk_object_calcancestors(cand);
            if(cand->obj.ancestors == NULL) return 0;
            array_concat(il, cand->obj.ancestors);
            array_pushptr(ol, il);
        }
    } else {
        cand = self->obj.parent;
        if(cand != NULL) {
            il = array_allocptr();
            if(cand->obj.ancestors == NULL) lk_object_calcancestors(cand);
            if(cand->obj.ancestors == NULL) return 0;
            array_concat(il, cand->obj.ancestors);
            array_pushptr(ol, il);
        }
    }
    if(LK_OBJ_HASPARENTS(self)) {
        il = array_allocptr();
        array_concat(il, LK_OBJ_PARENTS(self));
        array_pushptr(ol, il);
    } else {
        if(self->obj.parent != NULL) {
            il = array_allocptr();
            array_pushptr(il, self->obj.parent);
            array_pushptr(ol, il);
        }
    }
    startover:
    for(candi = 0, olc = LIST_COUNT(ol); candi < olc; candi ++) {
        il = LIST_ATPTR(ol, candi);
        if(LIST_COUNT(il) < 1) continue;
        cand = LIST_ATPTR(il, 0);
        for(j = 0; j < olc; j ++) {
            il = LIST_ATPTR(ol, j);
            for(k = 1, ilc = LIST_COUNT(il); k < ilc; k ++) {
                if(cand == LIST_ATPTR(il, k)) {
                    cand = NULL;
                    goto nextcandidate;
                }
            }
        }
        array_pushptr(newancs, cand);
        for(j = 0; j < olc; j ++) {
            il = LIST_ATPTR(ol, j);
            for(k = 0, ilc = LIST_COUNT(il); k < ilc; k ++) {
                if(cand == array_getptr(il, k)) array_removeptr(il, k --);
            }
        }
        goto startover;
        nextcandidate:;
    }
    for(j = 0; j < LIST_COUNT(ol); j ++) array_free(LIST_ATPTR(ol, j));
    array_free(ol);
    if(cand == NULL) return 0;
    else {
        array_t *oldancs = self->obj.ancestors;
        if(oldancs != NULL) array_free(oldancs);
        self->obj.ancestors = newancs;
        return 1;
    }
}

/* info */
#define FIND(nil, check) do { \
    while(1) { \
        check \
        if(self->obj.ancestors != NULL) goto checkancestors; \
        self = self->obj.parent; \
    } \
    checkancestors: { \
        array_t *ancs = self->obj.ancestors; \
        int i, c = LIST_COUNT(ancs); \
        for(i = 1; i < c; i ++) { \
            self = LIST_ATPTR(ancs, i); \
            check \
        } \
        return (nil); \
    } \
} while(0)
int lk_object_isa(lk_object_t *self, lk_object_t *t) {
    int dist = 1;
    FIND(0,
        if(self == t) return dist;
        dist ++;
    );
}
struct lk_slot *lk_object_getslot(lk_object_t *self, lk_object_t *k) {
    set_t *slots = self->obj.slots;
    setitem_t *item;
    if(slots == NULL || (item = set_get(slots, k)) == NULL) return NULL;
    return LK_SLOT(SETITEM_VALUEPTR(item));
}
struct lk_slot *lk_object_getslotfromany(lk_object_t *self, lk_object_t *k) {
    set_t *slots;
    setitem_t *si;
    FIND(NULL,
        if((slots = self->obj.slots) != NULL
        && (si = set_get(slots, k)) != NULL
        ) return LK_SLOT(SETITEM_VALUEPTR(si));
    );
}
lk_object_t *lk_object_getvaluefromslot(lk_object_t *self,
                                        struct lk_slot *slot) {
    if(LK_SLOT_GETTYPE(slot) == LK_SLOTTYPE_CFIELDLKOBJ) {
        lk_object_t *v = *(lk_object_t **)((ptrdiff_t)self + slot->value.coffset);
        return v != NULL ? v : LK_VM(self)->t_nil;
    } else {
        return slot->value.lkobj;
    }
}
int lk_object_hashcode(const void *k, int capa) {
    return array_hc(LIST(k)) % capa;
}
int lk_object_keycmp(const void *self, const void *other) {
    if(self == other) return 0;
    return !LIST_EQ(LIST(self), LIST(other));
}
