#include "object.h"
#include "ext.h"
#include "dict.h"
#include "fixnum.h"
#include "gc.h"
#include "list.h"

/* ext map - types */
static LK_OBJECT_DEFALLOCFUNC(alloc__object) {
    /*
    size_t d = self->co.tag->size - sizeof(lk_object_t);
    printf("d=%d\n", d);
    if(d > 0) memcpy((void *)((ptrdiff_t)self + sizeof(lk_object_t)),
    (void *)((ptrdiff_t)proto + sizeof(lk_object_t)), d);
     */
}
LK_EXT_DEFINIT(lk_object_extinittypes) {
    lk_object_t *o = vm->t_object = memory_alloc(sizeof(lk_object_t));
    o->co.tag = memory_alloc(sizeof(struct lk_tag));
    o->co.tag->refc = 1;
    o->co.tag->vm = vm;
    o->co.tag->size = sizeof(lk_object_t);
    o->co.tag->allocfunc = alloc__object;
    o->co.proto = NULL;
    list_pushptr(o->co.ancestors = list_allocptr(), o);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(DassignB__obj_str_obj) {
    lk_object_t *k = ARG(0);
    lk_object_t *v = ARG(1);
    struct lk_slot *slot = lk_object_getdef(self, k);
    if(slot == NULL) lk_vm_raisecstr(VM, "Cannot assign to %s without defining it first", k);
    slot = lk_object_setslot(self, k, slot->type, v);
    v = lk_object_getslot(self, slot);
    if(LK_OBJECT_ISFUNC(v)) {
        SETOPT(slot->opts, LK_SLOTVOAUTORUN);
        SETOPT(LK_FUNC(v)->cf.opts, LK_FUNCOASSIGNED);
        LK_FUNC(v)->cf.doc = env->caller->current->prev->comment;
    }
    RETURN(v);
}
static LK_EXT_DEFCFUNC(Ddefine_and_assignB__obj_str_obj_obj);
static LK_EXT_DEFCFUNC(Ddefine__obj_str_obj) {
    env->argc ++;
    list_pushptr(&env->stack, N);
    GOTO(Ddefine_and_assignB__obj_str_obj_obj);
}
static LK_EXT_DEFCFUNC(Ddefine_and_assignB__obj_str_obj) {
    env->argc ++;
    list_insertptr(&env->stack, 1, VM->t_object);
    GOTO(Ddefine_and_assignB__obj_str_obj_obj);
}
static LK_EXT_DEFCFUNC(Ddefine_and_assignB__obj_str_obj_obj) {
    lk_object_t *k = ARG(0);
    lk_object_t *v = ARG(2);
    struct lk_slot *slot;
    if(self->co.slots != NULL && set_get(self->co.slots, k) != NULL) {
        lk_vm_raisecstr(VM, "Cannot redefine %s", k);
    }
    slot = lk_object_setslot(self, k, ARG(1), v);
    v = lk_object_getslot(self, slot);
    if(LK_OBJECT_ISFUNC(v)) {
        SETOPT(slot->opts, LK_SLOTVOAUTORUN);
        SETOPT(LK_FUNC(v)->cf.opts, LK_FUNCOASSIGNED);
        LK_FUNC(v)->cf.doc = env->caller->current->prev->comment;
    }
    RETURN(v);
}
static LK_EXT_DEFCFUNC(Did__obj) {
    RETURN(lk_fi_new(VM, (int)self)); }
static LK_EXT_DEFCFUNC(Dretrieve__obj_str) {
    struct lk_slot *slot = lk_object_getdef(self, ARG(0));
    if(slot != NULL) RETURN(lk_object_getslot(self, slot));
    else RETURN(N);

}
static LK_EXT_DEFCFUNC(Dself__obj) {
    RETURN(self); }
static LK_EXT_DEFCFUNC(Dslots__obj) {
    lk_list_t *slots = lk_list_new(VM);
    if(self->co.slots != NULL) {
        SET_EACH(self->co.slots, i,
            list_pushptr(LIST(slots), (void *)i->key);
        );
    }
    RETURN(slots);
}
static LK_EXT_DEFCFUNC(alloc__obj) {
    RETURN(lk_object_alloc(self)); }
static LK_EXT_DEFCFUNC(also__obj_obj) {
    list_t *pars;
    if(LK_OBJECT_HASPARENTS(self)) {
        pars = LK_OBJECT_PARENTS(self);
    } else {
        pars = list_allocptr();
        list_pushptr(pars, self->co.proto);
        self->co.proto = LK_O((ptrdiff_t)pars | 1);
    }
    list_unshiftptr(pars, ARG(0));
    if(lk_object_calcancestors(self)) {
        RETURN(self);
    } else {
        printf("BUG: Throw proper ancestor error here\n");
        exit(EXIT_FAILURE);
    }
}
static LK_EXT_DEFCFUNC(ancestor__obj_obj) {
    RETURN(LK_OBJECT_ISTYPE(self, ARG(0)) ? VM->t_true : VM->t_false); }
static LK_EXT_DEFCFUNC(ancestors__obj) {
    if(self->co.ancestors != NULL || lk_object_calcancestors(self)) {
        RETURN(lk_list_newfromlist(VM, self->co.ancestors));
    } else {
        printf("BUG: Throw proper ancestor error here\n");
        exit(EXIT_FAILURE);
    }
}
static LK_EXT_DEFCFUNC(clone__obj) {
    RETURN(lk_object_clone(self)); }
static LK_EXT_DEFCFUNC(do__obj_f) {
    lk_kfunc_t *kf = LK_KFUNC(ARG(0));
    lk_frame_t *fr = lk_vm_prepevalfunc(VM);
    fr->first = fr->next = kf->first;
    fr->receiver = fr->self = self;
    fr->func = LK_O(kf);
    fr->returnto = NULL;
    fr->co.proto = LK_O(kf->frame);
    lk_vm_doevalfunc(VM);
    RETURN(self);
}
static LK_EXT_DEFCFUNC(import__obj_obj) {
    set_t *from = ARG(0)->co.slots;
    if(from != NULL) {
        set_t *to = self->co.slots;
        if(to == NULL) to = self->co.slots = set_alloc(
        sizeof(struct lk_slot), lk_object_hashcode, lk_object_keycmp);
        SET_EACH(from, i,
            *LK_SLOTV(set_set(to, i->key)) = *LK_SLOTV(SETITEM_VALUEPTR(i));
        );
    }
    RETURN(self);
}
static LK_EXT_DEFCFUNC(parents__obj) {
    if(LK_OBJECT_HASPARENTS(self)) {
        RETURN(lk_list_newfromlist(VM, LK_OBJECT_PARENTS(self)));
    } else {
        lk_list_t *ret = lk_list_new(VM);
        list_pushptr(LIST(ret), self->co.proto);
        RETURN(ret);
    }
}
static LK_EXT_DEFCFUNC(proto__obj) {
    if(LK_OBJECT_HASPARENTS(self)) {
        list_t *pars = LK_OBJECT_PARENTS(self);
        RETURN(LIST_COUNT(pars) > 0 ? list_getptr(pars, -1) : N);
    }
    RETURN(self->co.proto);
}
static LK_EXT_DEFCFUNC(with__obj_f) {
    do__obj_f(lk_object_addref(LK_O(env), lk_object_alloc(self)), currslot, env);
}
LK_EXT_DEFINIT(lk_object_extinitfuncs) {
    lk_object_t *obj = vm->t_object, *str = vm->t_string, *f = vm->t_func;
    lk_ext_global("Object", obj);
    lk_ext_cfunc(obj, ".", Dself__obj, NULL);
    lk_ext_cfunc(obj, ".assign!", DassignB__obj_str_obj, str, obj, NULL);
    lk_ext_cfunc(obj, ".define!", Ddefine__obj_str_obj, str, obj, NULL);
    lk_ext_cfunc(obj, ".define and assign!",
                 Ddefine_and_assignB__obj_str_obj, str, obj, NULL);
    lk_ext_cfunc(obj, ".define and assign!",
                 Ddefine_and_assignB__obj_str_obj_obj, str, obj, obj, NULL);
    lk_ext_cfunc(obj, ".id", Did__obj, NULL);
    lk_ext_cfunc(obj, ".retrieve", Dretrieve__obj_str, str, NULL);
    lk_ext_cfunc(obj, ".self", Dself__obj, NULL);
    lk_ext_cfunc(obj, ".slots", Dslots__obj, NULL);
    lk_ext_cfunc(obj, "alloc", alloc__obj, NULL);
    lk_ext_cfunc(obj, "also", also__obj_obj, obj, NULL);
    lk_ext_cfunc(obj, "ancestor?", ancestor__obj_obj, obj, NULL);
    lk_ext_cfunc(obj, "ancestors", ancestors__obj, NULL);
    lk_ext_cfunc(obj, "clone", clone__obj, NULL);
    lk_ext_cfunc(obj, "do", do__obj_f, f, NULL);
    lk_ext_cfunc(obj, "import", import__obj_obj, obj, NULL);
    lk_ext_cfunc(obj, "parents", parents__obj, NULL);
    lk_ext_cfunc(obj, "proto", proto__obj, NULL);
    lk_ext_cfunc(obj, "with", with__obj_f, f, NULL);
}

/* new */
static struct lk_tag *tag_clone(struct lk_tag *self) {
    struct lk_tag *clone = memory_alloc(sizeof(struct lk_tag));
    memcpy(clone, self, sizeof(struct lk_tag));
    clone->refc = 1;
    return clone;
}
lk_object_t *lk_object_allocwithsize(lk_object_t *proto, size_t s) {
    lk_gc_t *gc = LK_VM(proto)->gc;
    lk_object_t *self = memory_alloc(s);
    struct lk_tag *tag = proto->co.tag;
    if(tag->size == s) tag->refc ++;
    else (tag = tag_clone(tag))->size = s;
    self->co.proto = proto;
    self->co.tag = tag;
    if(tag->allocfunc != NULL) tag->allocfunc(self, proto);
    if(gc != NULL) {
        gc->newvalues ++;
        lk_objgroup_insert(gc->unused, self);
    }
    return self;
}
lk_object_t *lk_object_alloc(lk_object_t *proto) {
    return lk_object_allocwithsize(proto, proto->co.tag->size);
}
lk_object_t *lk_object_clone(lk_object_t *self) {
    lk_object_t *c = lk_object_alloc(LK_OBJECT_PROTO(self));
    if(c->co.tag->allocfunc != NULL) c->co.tag->allocfunc(c, self);
    return c;
}
void lk_object_justfree(lk_object_t *self) {
    struct lk_tag *tag = self->co.tag;
    if(tag->freefunc != NULL) tag->freefunc(self);
    if(LK_OBJECT_HASPARENTS(self)) list_free(LK_OBJECT_PARENTS(self));
    if(self->co.ancestors != NULL) list_free(self->co.ancestors);
    if(self->co.slots != NULL) set_free(self->co.slots);
    if(-- tag->refc < 1) memory_free(tag);
    memory_free(self);
}
void lk_object_free(lk_object_t *self) {
    lk_objgroup_remove(self);
    lk_object_justfree(self);
}

/* update - tag */
#define LK_OBJECT_IMPLTAGSETTER(t, field) \
LK_OBJECT_DEFTAGSETTER(t, field) { \
    struct lk_tag *tag = self->co.tag; \
    if(tag->field != field) { \
        if(tag->refc > 1) { \
            tag->refc --; \
            tag = self->co.tag = tag_clone(tag); \
        } \
        tag->field = field; \
    } \
} LK_OBJECT_DEFTAGSETTER(t, field)
LK_OBJECT_IMPLTAGSETTER(lk_tagallocfunc_t *, allocfunc);
LK_OBJECT_IMPLTAGSETTER(lk_tagmarkfunc_t *, markfunc);
LK_OBJECT_IMPLTAGSETTER(lk_tagfreefunc_t *, freefunc);

/* update */
struct lk_slot *lk_object_setslot(lk_object_t *self, lk_object_t *k, lk_object_t *t, lk_object_t *v) {
    set_t *slots = self->co.slots;
    struct lk_slot *slot;
    if(slots == NULL) slots = self->co.slots = set_alloc(
    sizeof(struct lk_slot), lk_object_hashcode, lk_object_keycmp);
    else {
        setitem_t *si = set_get(slots, k);
        if(si != NULL) {
            lk_object_t *oldval;
            slot = LK_SLOTV(SETITEM_VALUEPTR(si));
            slot->type = t;
            oldval = lk_object_getslot(self, slot);
            if(LK_OBJECT_ISFUNC(oldval)) {
                v = LK_O(lk_func_combine(LK_FUNC(oldval), LK_FUNC(v)));
            }
            lk_object_setslotvalue(self, slot, k, v);
            return slot;
        }
    }
    slot = LK_SLOTV(set_set(slots, k));
    slot->type = t;
    lk_object_setslotvalue(self, slot, k, v);
    return slot;
}
struct lk_slot *lk_object_setslotbycstr(lk_object_t *self, const char *k, lk_object_t *t, lk_object_t *v) {
    return lk_object_setslot(self,
    LK_O(lk_string_newfromcstr(LK_VM(self), k)), t, v);
}
void lk_object_setslotvalue(lk_object_t *self, struct lk_slot *slot, lk_object_t *k, lk_object_t *v) {
    lk_vm_t *vm = LK_VM(self);
    if(v == NULL) v = vm->t_unknown;
    if(v == vm->t_unknown || LK_OBJECT_ISTYPE(v, slot->type)) {
        {
            /* on-assign trigger */
            /*
            lk_string_t *kt = lk_string_newfromlist(vm, LIST(k));
            struct lk_slot *ou;
            list_resizeitem(LIST(kt), LIST(vm->str_onassign));
            list_concat(LIST(kt), LIST(vm->str_onassign));
            ou = lk_object_getdef(self, LK_O(kt));
            lk_object_free(LK_O(kt));
            if(ou != NULL) {
                lk_kfunc_t *kf = LK_KFUNC(lk_object_getslot(self, ou));
                if(LK_OBJECT_ISFUNC(LK_O(kf))) {
                    lk_frame_t *fr = lk_vm_prepevalfunc(vm);
                    lk_frame_stackpush(fr, v);
                    kf = LK_KFUNC(lk_func_match(LK_FUNC(kf), fr, self));
                    if(kf == NULL) {
                        vm->currframe = vm->currframe->caller;
                    } else if(CHKOPT(kf->cf.opts, LK_FUNCORUNNING)) {
                        string_print(LIST(k), stdout);
                        printf("\n");
                        lk_vm_raisecstr(vm,
                        "Cannot assign to var while running on-assign");
                    } else {
                        if(!(slot->opts & LK_SLOTVOCFIELD)
                        && slot->v.value == NULL) slot->v.value = vm->t_unknown;
                        SETOPT(kf->cf.opts, LK_FUNCORUNNING);
                        fr->first = fr->next = kf->first;
                        fr->receiver = fr->self = self;
                        fr->func = LK_O(kf);
                        fr->returnto = NULL;
                        fr->co.proto = LK_O(kf->frame);
                        lk_vm_doevalfunc(vm);
                        CLROPT(kf->cf.opts, LK_FUNCORUNNING);
                        v = lk_frame_stackpeek(fr);
                        if(v == NULL) v = vm->t_unknown;
                        slot = LK_SLOTV(SETITEM_VALUEPTR(
                        set_get(self->co.slots, k)));
                    }
                }
            }
            */
        }
        lk_object_addref(self, v);
        if(slot->opts & LK_SLOTVOCFIELD) {
            if(v == vm->t_unknown) v = NULL;
            *(lk_object_t **)((ptrdiff_t)self + slot->v.offset) = v;
        } else {
            slot->v.value = v;
        }
    } else {
        printf("type mismatch!\n");
        exit(EXIT_FAILURE);
    }
}
int lk_object_calcancestors(lk_object_t *self) {
    lk_object_t *cand = NULL;
    list_t *pars;
    list_t *il, *newancs = list_allocptr(), *ol = list_allocptr();
    int candi, j, k, olc, ilc;
    il = list_allocptr();
    list_pushptr(il, self);
    list_pushptr(ol, il);
    if(LK_OBJECT_HASPARENTS(self)) {
        pars = LK_OBJECT_PARENTS(self);
        for(candi = 0; candi < LIST_COUNT(pars); candi ++) {
            cand = LIST_ATPTR(pars, candi);
            il = list_allocptr();
            if(cand->co.ancestors == NULL) lk_object_calcancestors(cand);
            if(cand->co.ancestors == NULL) return 0;
            list_concat(il, cand->co.ancestors);
            list_pushptr(ol, il);
        }
    } else {
        cand = self->co.proto;
        if(cand != NULL) {
            il = list_allocptr();
            if(cand->co.ancestors == NULL) lk_object_calcancestors(cand);
            if(cand->co.ancestors == NULL) return 0;
            list_concat(il, cand->co.ancestors);
            list_pushptr(ol, il);
        }
    }
    if(LK_OBJECT_HASPARENTS(self)) {
        il = list_allocptr();
        list_concat(il, LK_OBJECT_PARENTS(self));
        list_pushptr(ol, il);
    } else {
        if(self->co.proto != NULL) {
            il = list_allocptr();
            list_pushptr(il, self->co.proto);
            list_pushptr(ol, il);
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
        list_pushptr(newancs, cand);
        for(j = 0; j < olc; j ++) {
            il = LIST_ATPTR(ol, j);
            for(k = 0, ilc = LIST_COUNT(il); k < ilc; k ++) {
                if(cand == list_getptr(il, k)) list_removeptr(il, k --);
            }
        }
        goto startover;
        nextcandidate:;
    }
    for(j = 0; j < LIST_COUNT(ol); j ++) list_free(LIST_ATPTR(ol, j));
    list_free(ol);
    if(cand == NULL) return 0;
    else {
        list_t *oldancs = self->co.ancestors;
        if(oldancs != NULL) list_free(oldancs);
        self->co.ancestors = newancs;
        return 1;
    }
}

/* info */
#define FIND(nil, check) do { \
    while(1) { \
        check \
        if(self->co.ancestors != NULL) goto checkancestors; \
        self = self->co.proto; \
    } \
    checkancestors: { \
        list_t *ancs = self->co.ancestors; \
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
struct lk_slot *lk_object_getdef(lk_object_t *self, lk_object_t *k) {
    set_t *slots;
    setitem_t *si;
    FIND(NULL,
        if((slots = self->co.slots) != NULL
        && (si = set_get(slots, k)) != NULL
        ) return LK_SLOTV(SETITEM_VALUEPTR(si));
    );
}
lk_object_t *lk_object_getslot(lk_object_t *self, struct lk_slot *slot) {
    if(slot->opts & LK_SLOTVOCFIELD) {
        lk_object_t *v = *(lk_object_t **)((ptrdiff_t)self + slot->v.offset);
        return v != NULL ? v : LK_VM(self)->t_unknown;
    } else {
        return slot->v.value;
    }
}
int lk_object_hashcode(const void *k, int capa) {
    return list_hc(LIST(k)) % capa;
}
int lk_object_keycmp(const void *self, const void *other) {
    if(self == other) return 0;
    return !LIST_EQ(LIST(self), LIST(other));
}
