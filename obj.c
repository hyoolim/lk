#include "obj.h"
#include "ext.h"
#include "dict.h"
#include "fixnum.h"
#include "gc.h"
#include "list.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_obj_extinittypes) {
    lk_obj_t *o = vm->t_obj = memory_alloc(sizeof(lk_obj_t));
    o->obj.tag = memory_alloc(sizeof(struct lk_tag));
    o->obj.tag->refc = 1;
    o->obj.tag->vm = vm;
    o->obj.tag->size = sizeof(lk_obj_t);
    o->obj.proto = NULL;
    list_pushptr(o->obj.ancestors = list_allocptr(), o);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(Ddefine_and_assignB__obj_str_obj_obj);
static LK_EXT_DEFCFUNC(Ddefine__obj_str_obj) {
    env->argc ++;
    list_pushptr(&env->stack, N);
    GOTO(Ddefine_and_assignB__obj_str_obj_obj);
}
static LK_EXT_DEFCFUNC(Ddefine_and_assignB__obj_str_obj) {
    env->argc ++;
    list_insertptr(&env->stack, 1, VM->t_obj);
    GOTO(Ddefine_and_assignB__obj_str_obj_obj);
}
static LK_EXT_DEFCFUNC(Ddefine_and_assignB__obj_str_obj_obj) {
    lk_obj_t *k = ARG(0);
    lk_obj_t *v = ARG(2);
    struct lk_slot *slot;
    if(self->obj.slots != NULL && set_get(self->obj.slots, k) != NULL) {
        lk_vm_raisecstr(VM, "Cannot redefine %s", k);
    }
    slot = lk_obj_setslot(self, k, ARG(1), v);
    v = lk_obj_getvaluefromslot(self, slot);
    if(LK_OBJ_ISFUNC(v)) {
        LK_SLOT_SETOPTION(slot, LK_SLOTOPTION_AUTOSEND);
        SETOPT(LK_FUNC(v)->cf.opts, LK_FUNCOASSIGNED);
        LK_FUNC(v)->cf.doc = env->caller->current->prev->comment;
    }
    RETURN(v);
}
static LK_EXT_DEFCFUNC(Did__obj) {
    RETURN(lk_fi_new(VM, (int)self)); }
static LK_EXT_DEFCFUNC(Dretrieve__obj_str) {
    struct lk_slot *slot = lk_obj_getslotfromany(self, ARG(0));
    if(slot != NULL) RETURN(lk_obj_getvaluefromslot(self, slot));
    else RETURN(N);

}
static LK_EXT_DEFCFUNC(Dself__obj) {
    RETURN(self); }
static LK_EXT_DEFCFUNC(Dslots__obj) {
    lk_list_t *slots = lk_list_new(VM);
    if(self->obj.slots != NULL) {
        SET_EACH(self->obj.slots, i,
            list_pushptr(LIST(slots), (void *)i->key);
        );
    }
    RETURN(slots);
}
static LK_EXT_DEFCFUNC(alloc__obj) {
    RETURN(lk_obj_alloc(self)); }
static LK_EXT_DEFCFUNC(also__obj_obj) {
    list_t *pars;
    if(LK_OBJ_HASPARENTS(self)) {
        pars = LK_OBJ_PARENTS(self);
    } else {
        pars = list_allocptr();
        list_pushptr(pars, self->obj.proto);
        self->obj.proto = LK_OBJ((ptrdiff_t)pars | 1);
    }
    list_unshiftptr(pars, ARG(0));
    if(lk_obj_calcancestors(self)) {
        RETURN(self);
    } else {
        printf("BUG: Throw proper ancestor error here\n");
        exit(EXIT_FAILURE);
    }
}
static LK_EXT_DEFCFUNC(ancestor__obj_obj) {
    RETURN(LK_OBJ_ISTYPE(self, ARG(0)) ? VM->t_true : VM->t_false); }
static LK_EXT_DEFCFUNC(ancestors__obj) {
    if(self->obj.ancestors != NULL || lk_obj_calcancestors(self)) {
        RETURN(lk_list_newfromlist(VM, self->obj.ancestors));
    } else {
        printf("BUG: Throw proper ancestor error here\n");
        exit(EXIT_FAILURE);
    }
}
static LK_EXT_DEFCFUNC(clone__obj) {
    RETURN(lk_obj_clone(self)); }
static LK_EXT_DEFCFUNC(do__obj_f) {
    lk_kfunc_t *kf = LK_KFUNC(ARG(0));
    lk_frame_t *fr = lk_vm_prepevalfunc(VM);
    fr->first = fr->next = kf->first;
    fr->receiver = fr->self = self;
    fr->func = LK_OBJ(kf);
    fr->returnto = NULL;
    fr->obj.proto = LK_OBJ(kf->frame);
    lk_vm_doevalfunc(VM);
    RETURN(self);
}
static LK_EXT_DEFCFUNC(import__obj_obj) {
    set_t *from = ARG(0)->obj.slots;
    if(from != NULL) {
        set_t *to = self->obj.slots;
        if(to == NULL) to = self->obj.slots = set_alloc(
        sizeof(struct lk_slot), lk_obj_hashcode, lk_obj_keycmp);
        SET_EACH(from, i,
            *LK_SLOT(set_set(to, i->key)) = *LK_SLOT(SETITEM_VALUEPTR(i));
        );
    }
    RETURN(self);
}
static LK_EXT_DEFCFUNC(parents__obj) {
    if(LK_OBJ_HASPARENTS(self)) {
        RETURN(lk_list_newfromlist(VM, LK_OBJ_PARENTS(self)));
    } else {
        lk_list_t *ret = lk_list_new(VM);
        list_pushptr(LIST(ret), self->obj.proto);
        RETURN(ret);
    }
}
static LK_EXT_DEFCFUNC(proto__obj) {
    if(LK_OBJ_HASPARENTS(self)) {
        list_t *pars = LK_OBJ_PARENTS(self);
        RETURN(LIST_COUNT(pars) > 0 ? list_getptr(pars, -1) : N);
    }
    RETURN(self->obj.proto);
}
static LK_EXT_DEFCFUNC(with__obj_f) {
    do__obj_f(lk_obj_addref(LK_OBJ(env), lk_obj_alloc(self)), env);
}
LK_EXT_DEFINIT(lk_obj_extinitfuncs) {
    lk_obj_t *obj = vm->t_obj, *str = vm->t_string, *f = vm->t_func;
    lk_ext_global("Object", obj);
    lk_ext_cfunc(obj, ".", Dself__obj, NULL);
    lk_ext_cfunc(obj, "define!", Ddefine__obj_str_obj, str, obj, NULL);
    lk_ext_cfunc(obj, "define=",
                 Ddefine_and_assignB__obj_str_obj, str, obj, NULL);
    lk_ext_cfunc(obj, "define=",
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
lk_obj_t *lk_obj_allocwithsize(lk_obj_t *proto, size_t s) {
    lk_gc_t *gc = LK_VM(proto)->gc;
    lk_obj_t *self = memory_alloc(s);
    struct lk_tag *tag = proto->obj.tag;
    if(tag->size == s) tag->refc ++;
    else (tag = tag_clone(tag))->size = s;
    self->obj.proto = proto;
    self->obj.tag = tag;
    if(tag->allocfunc != NULL) tag->allocfunc(self, proto);
    if(gc != NULL) {
        gc->newvalues ++;
        lk_objgroup_insert(gc->unused, self);
    }
    return self;
}
lk_obj_t *lk_obj_alloc(lk_obj_t *proto) {
    return lk_obj_allocwithsize(proto, proto->obj.tag->size);
}
lk_obj_t *lk_obj_clone(lk_obj_t *self) {
    lk_obj_t *c = lk_obj_alloc(LK_OBJ_PROTO(self));
    if(c->obj.tag->allocfunc != NULL) c->obj.tag->allocfunc(c, self);
    return c;
}
void lk_obj_justfree(lk_obj_t *self) {
    struct lk_tag *tag = self->obj.tag;
    if(tag->freefunc != NULL) tag->freefunc(self);
    if(LK_OBJ_HASPARENTS(self)) list_free(LK_OBJ_PARENTS(self));
    if(self->obj.ancestors != NULL) list_free(self->obj.ancestors);
    if(self->obj.slots != NULL) set_free(self->obj.slots);
    if(-- tag->refc < 1) memory_free(tag);
    memory_free(self);
}
void lk_obj_free(lk_obj_t *self) {
    lk_objgroup_remove(self);
    lk_obj_justfree(self);
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
struct lk_slot *lk_obj_setslot(lk_obj_t *self, lk_obj_t *k,
                                  lk_obj_t *check, lk_obj_t *v) {
    struct lk_slot *slot = lk_obj_getslot(self, k);
    if(slot == NULL) {
        uint32_t first = list_getuchar(LIST(k), 0);
        if(self->obj.slots == NULL) {
            self->obj.slots = set_alloc(sizeof(struct lk_slot),
                                       lk_obj_hashcode,
                                       lk_obj_keycmp);
        }
        slot = LK_SLOT(set_set(self->obj.slots, k));
        slot->check = check;
        if('A' <= first && first <= 'Z') {
            LK_SLOT_SETOPTION(slot, LK_SLOTOPTION_READONLY);
            lk_obj_setslot(v, LK_OBJ(LK_VM(self)->str_type),
                              LK_OBJ(LK_VM(self)->t_string), LK_OBJ(k));
        }
    }
    lk_obj_setvalueonslot(self, slot, v);
    return slot;
}
struct lk_slot *lk_obj_setslotbycstr(lk_obj_t *self, const char *k,
                                        lk_obj_t *check, lk_obj_t *v) {
    return lk_obj_setslot(self,
    LK_OBJ(lk_string_newfromcstr(LK_VM(self), k)), check, v);
}
void lk_obj_setvalueonslot(lk_obj_t *self, struct lk_slot *slot,
                              lk_obj_t *v) {
    lk_vm_t *vm = LK_VM(self);
    if(v == NULL) v = vm->t_unknown;
    if(v == vm->t_unknown || LK_OBJ_ISTYPE(v, slot->check)) {
        lk_obj_addref(self, v);
        if(LK_SLOT_GETTYPE(slot) == LK_SLOTTYPE_CFIELDLKOBJ) {
            if(v == vm->t_unknown) v = NULL;
            *(lk_obj_t **)((ptrdiff_t)self + slot->value.coffset) = v;
        } else {
            slot->value.lkobj = v;
        }
    } else {
        printf("type mismatch!\n");
        exit(EXIT_FAILURE);
    }
}
int lk_obj_calcancestors(lk_obj_t *self) {
    lk_obj_t *cand = NULL;
    list_t *pars;
    list_t *il, *newancs = list_allocptr(), *ol = list_allocptr();
    int candi, j, k, olc, ilc;
    il = list_allocptr();
    list_pushptr(il, self);
    list_pushptr(ol, il);
    if(LK_OBJ_HASPARENTS(self)) {
        pars = LK_OBJ_PARENTS(self);
        for(candi = 0; candi < LIST_COUNT(pars); candi ++) {
            cand = LIST_ATPTR(pars, candi);
            il = list_allocptr();
            if(cand->obj.ancestors == NULL) lk_obj_calcancestors(cand);
            if(cand->obj.ancestors == NULL) return 0;
            list_concat(il, cand->obj.ancestors);
            list_pushptr(ol, il);
        }
    } else {
        cand = self->obj.proto;
        if(cand != NULL) {
            il = list_allocptr();
            if(cand->obj.ancestors == NULL) lk_obj_calcancestors(cand);
            if(cand->obj.ancestors == NULL) return 0;
            list_concat(il, cand->obj.ancestors);
            list_pushptr(ol, il);
        }
    }
    if(LK_OBJ_HASPARENTS(self)) {
        il = list_allocptr();
        list_concat(il, LK_OBJ_PARENTS(self));
        list_pushptr(ol, il);
    } else {
        if(self->obj.proto != NULL) {
            il = list_allocptr();
            list_pushptr(il, self->obj.proto);
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
        list_t *oldancs = self->obj.ancestors;
        if(oldancs != NULL) list_free(oldancs);
        self->obj.ancestors = newancs;
        return 1;
    }
}

/* info */
#define FIND(nil, check) do { \
    while(1) { \
        check \
        if(self->obj.ancestors != NULL) goto checkancestors; \
        self = self->obj.proto; \
    } \
    checkancestors: { \
        list_t *ancs = self->obj.ancestors; \
        int i, c = LIST_COUNT(ancs); \
        for(i = 1; i < c; i ++) { \
            self = LIST_ATPTR(ancs, i); \
            check \
        } \
        return (nil); \
    } \
} while(0)
int lk_obj_isa(lk_obj_t *self, lk_obj_t *t) {
    int dist = 1;
    FIND(0,
        if(self == t) return dist;
        dist ++;
    );
}
struct lk_slot *lk_obj_getslot(lk_obj_t *self, lk_obj_t *k) {
    set_t *slots = self->obj.slots;
    setitem_t *item;
    if(slots == NULL || (item = set_get(slots, k)) == NULL) return NULL;
    return LK_SLOT(SETITEM_VALUEPTR(item));
}
struct lk_slot *lk_obj_getslotfromany(lk_obj_t *self, lk_obj_t *k) {
    set_t *slots;
    setitem_t *si;
    FIND(NULL,
        if((slots = self->obj.slots) != NULL
        && (si = set_get(slots, k)) != NULL
        ) return LK_SLOT(SETITEM_VALUEPTR(si));
    );
}
lk_obj_t *lk_obj_getvaluefromslot(lk_obj_t *self,
                                        struct lk_slot *slot) {
    if(LK_SLOT_GETTYPE(slot) == LK_SLOTTYPE_CFIELDLKOBJ) {
        lk_obj_t *v = *(lk_obj_t **)((ptrdiff_t)self + slot->value.coffset);
        return v != NULL ? v : LK_VM(self)->t_unknown;
    } else {
        return slot->value.lkobj;
    }
}
int lk_obj_hashcode(const void *k, int capa) {
    return list_hc(LIST(k)) % capa;
}
int lk_obj_keycmp(const void *self, const void *other) {
    if(self == other) return 0;
    return !LIST_EQ(LIST(self), LIST(other));
}
