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
    lk_object_t *o = vm->t_object = pt_memory_alloc(sizeof(lk_object_t));
    o->co.tag = pt_memory_alloc(sizeof(struct lk_tag));
    o->co.tag->refc = 1;
    o->co.tag->vm = vm;
    o->co.tag->size = sizeof(lk_object_t);
    o->co.tag->allocfunc = alloc__object;
    o->co.proto = NULL;
    pt_list_pushptr(o->co.ancestors = pt_list_allocptr(), o);
}

/* ext map - funcs */
#define CHECKDEF(self, k) do { \
    if((self)->co.slots != NULL \
    && pt_set_get((self)->co.slots, (k)) != NULL) { \
        pt_string_print(LIST(k), stdout); \
        lk_vm_raisecstr(VM, "Cannot assign to %s without defining it first", k); \
    } \
} while(0)
static LK_EXT_DEFCFUNC(DassignB__obj_str_obj) {
    lk_object_t *k = ARG(0);
    lk_object_t *v = ARG(1);
    struct lk_slotv *sv = lk_object_getdef(self, k);
    if(sv == NULL) lk_vm_raisecstr(VM, "Cannot assign to %s without defining it first", k);
    sv = lk_object_setslot(self, k, sv->type, v);
    v = lk_object_getslotv(self, sv);
    if(LK_OBJECT_ISFUNC(v)) {
        SETOPT(sv->opts, LK_SLOTVOPT_AUTORUN);
        SETOPT(LK_FUNC(v)->cf.opts, LK_FUNCOPT_ASSIGNED);
        LK_FUNC(v)->cf.doc = env->caller->current->prev->comment;
    }
    RETURN(v);
}
static LK_EXT_DEFCFUNC(Ddefine__obj_str_obj) {
    lk_object_t *k = ARG(0);
    lk_object_t *t = ARG(1);
    CHECKDEF(self, k);
    lk_object_setslot(self, k, t, N);
    RETURN(t);
}
static LK_EXT_DEFCFUNC(Ddefine_assignB__obj_str_obj) {
    lk_object_t *k = ARG(0);
    lk_object_t *t = VM->t_object;
    lk_object_t *v = ARG(1);
    struct lk_slotv *sv;
    CHECKDEF(self, k);
    sv = lk_object_setslot(self, k, t, v);
    v = lk_object_getslotv(self, sv);
    if(LK_OBJECT_ISFUNC(v)) {
        SETOPT(sv->opts, LK_SLOTVOPT_AUTORUN);
        SETOPT(LK_FUNC(v)->cf.opts, LK_FUNCOPT_ASSIGNED);
        LK_FUNC(v)->cf.doc = env->caller->current->prev->comment;
    }
    RETURN(v);
}
static LK_EXT_DEFCFUNC(Ddefine_assignB__obj_str_obj_obj) {
    lk_object_t *k = ARG(0);
    lk_object_t *v = ARG(2);
    struct lk_slotv *sv;
    CHECKDEF(self, k);
    sv = lk_object_setslot(self, k, ARG(1), v);
    v = lk_object_getslotv(self, sv);
    if(LK_OBJECT_ISFUNC(v)) {
        SETOPT(sv->opts, LK_SLOTVOPT_AUTORUN);
        SETOPT(LK_FUNC(v)->cf.opts, LK_FUNCOPT_ASSIGNED);
        LK_FUNC(v)->cf.doc = env->caller->current->prev->comment;
    }
    RETURN(v);
}
static LK_EXT_DEFCFUNC(Did__obj) {
    RETURN(lk_fi_new(VM, (int)self)); }
static LK_EXT_DEFCFUNC(Dretrieve__obj_str) {
    struct lk_slotv *slot = lk_object_getdef(self, ARG(0));
    if(slot != NULL) RETURN(lk_object_getslotv(self, slot));
    else RETURN(N);

}
static LK_EXT_DEFCFUNC(Dself__obj) {
    RETURN(self); }
static LK_EXT_DEFCFUNC(Dslots__obj) {
    lk_list_t *slots = lk_list_new(VM);
    if(self->co.slots != NULL) {
        PT_SET_EACH(self->co.slots, i,
            pt_list_pushptr(LIST(slots), (void *)i->key);
        );
    }
    RETURN(slots);
}
static LK_EXT_DEFCFUNC(alloc__obj) {
    RETURN(lk_object_alloc(self)); }
static LK_EXT_DEFCFUNC(also__obj_obj) {
    pt_list_t *pars;
    if(LK_OBJECT_HASPARENTS(self)) {
        pars = LK_OBJECT_PARENTS(self);
    } else {
        pars = pt_list_allocptr();
        pt_list_pushptr(pars, self->co.proto);
        self->co.proto = LK_O((ptrdiff_t)pars | 1);
    }
    pt_list_unshiftptr(pars, ARG(0));
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
    pt_set_t *from = ARG(0)->co.slots;
    if(from != NULL) {
        pt_set_t *to = self->co.slots;
        if(to == NULL) to = self->co.slots = pt_set_alloc(
        sizeof(struct lk_slotv), lk_object_hashcode, lk_object_keycmp);
        PT_SET_EACH(from, i,
            *LK_SLOTV(pt_set_set(to, i->key)) = *LK_SLOTV(PT_SETITEM_VALUEPTR(i));
        );
    }
    RETURN(self);
}
static LK_EXT_DEFCFUNC(parents__obj) {
    if(LK_OBJECT_HASPARENTS(self)) {
        RETURN(lk_list_newfromlist(VM, LK_OBJECT_PARENTS(self)));
    } else {
        lk_list_t *ret = lk_list_new(VM);
        pt_list_pushptr(LIST(ret), self->co.proto);
        RETURN(ret);
    }
}
static LK_EXT_DEFCFUNC(proto__obj) {
    if(LK_OBJECT_HASPARENTS(self)) {
        pt_list_t *pars = LK_OBJECT_PARENTS(self);
        RETURN(PT_LIST_COUNT(pars) > 0 ? pt_list_getptr(pars, -1) : N);
    }
    RETURN(self->co.proto);
}
static LK_EXT_DEFCFUNC(with__obj_f) {
    do__obj_f(lk_object_addref(LK_O(env), lk_object_alloc(self)), env);
}
LK_EXT_DEFINIT(lk_object_extinitfuncs) {
    lk_object_t *obj = vm->t_object, *str = vm->t_string, *f = vm->t_func;
    lk_ext_global("Object", obj);
    lk_ext_cfunc(obj, ".", Dself__obj, NULL);
    lk_ext_cfunc(obj, ".assign!", DassignB__obj_str_obj, str, obj, NULL);
    lk_ext_cfunc(obj, ".define!", Ddefine__obj_str_obj, str, obj, NULL);
    lk_ext_cfunc(obj, ".define_assign!", Ddefine_assignB__obj_str_obj,
                                          str, obj, NULL);
    lk_ext_cfunc(obj, ".define_assign!", Ddefine_assignB__obj_str_obj_obj,
                                          str, obj, obj, NULL);
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
    struct lk_tag *clone = pt_memory_alloc(sizeof(struct lk_tag));
    memcpy(clone, self, sizeof(struct lk_tag));
    clone->refc = 1;
    return clone;
}
lk_object_t *lk_object_allocwithsize(lk_object_t *proto, size_t s) {
    lk_gc_t *gc = LK_VM(proto)->gc;
    lk_object_t *self = pt_memory_alloc(s);
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
    if(LK_OBJECT_HASPARENTS(self)) pt_list_free(LK_OBJECT_PARENTS(self));
    if(self->co.ancestors != NULL) pt_list_free(self->co.ancestors);
    if(self->co.slots != NULL) pt_set_free(self->co.slots);
    if(-- tag->refc < 1) pt_memory_free(tag);
    pt_memory_free(self);
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
struct lk_slotv *lk_object_setslot(lk_object_t *self, lk_object_t *k, lk_object_t *t, lk_object_t *v) {
    pt_set_t *slots = self->co.slots;
    struct lk_slotv *sv;
    if(slots == NULL) slots = self->co.slots = pt_set_alloc(
    sizeof(struct lk_slotv), lk_object_hashcode, lk_object_keycmp);
    else {
        pt_setitem_t *si = pt_set_get(slots, k);
        if(si != NULL) {
            lk_object_t *oldval;
            sv = LK_SLOTV(PT_SETITEM_VALUEPTR(si));
            sv->type = t;
            oldval = lk_object_getslotv(self, sv);
            if(LK_OBJECT_ISFUNC(oldval)) {
                v = LK_O(lk_func_combine(LK_FUNC(oldval), LK_FUNC(v)));
            }
            lk_object_setslotv(self, sv, k, v);
            return sv;
        }
    }
    sv = LK_SLOTV(pt_set_set(slots, k));
    sv->type = t;
    lk_object_setslotv(self, sv, k, v);
    return sv;
}
struct lk_slotv *lk_object_setslotbycstr(lk_object_t *self, const char *k, lk_object_t *t, lk_object_t *v) {
    return lk_object_setslot(self,
    LK_O(lk_string_newfromcstr(LK_VM(self), k)), t, v);
}
void lk_object_setslotv(lk_object_t *self, struct lk_slotv *slotv, lk_object_t *k, lk_object_t *v) {
    lk_vm_t *vm = LK_VM(self);
    if(v == NULL) v = vm->t_unknown;
    if(v == vm->t_unknown || LK_OBJECT_ISTYPE(v, slotv->type)) {
        {
            /* on-assign trigger */
            /*
            lk_string_t *kt = lk_string_newfromlist(vm, LIST(k));
            struct lk_slotv *ou;
            pt_list_resizeitem(LIST(kt), LIST(vm->str_onassign));
            pt_list_concat(LIST(kt), LIST(vm->str_onassign));
            ou = lk_object_getdef(self, LK_O(kt));
            lk_object_free(LK_O(kt));
            if(ou != NULL) {
                lk_kfunc_t *kf = LK_KFUNC(lk_object_getslotv(self, ou));
                if(LK_OBJECT_ISFUNC(LK_O(kf))) {
                    lk_frame_t *fr = lk_vm_prepevalfunc(vm);
                    lk_frame_stackpush(fr, v);
                    kf = LK_KFUNC(lk_func_match(LK_FUNC(kf), fr, self));
                    if(kf == NULL) {
                        vm->currframe = vm->currframe->caller;
                    } else if(CHKOPT(kf->cf.opts, LK_FUNCOPT_RUNNING)) {
                        pt_string_print(LIST(k), stdout);
                        printf("\n");
                        lk_vm_raisecstr(vm,
                        "Cannot assign to var while running on-assign");
                    } else {
                        if(!(slotv->opts & LK_SLOTVOPT_CFIELD)
                        && slotv->v.value == NULL) slotv->v.value = vm->t_unknown;
                        SETOPT(kf->cf.opts, LK_FUNCOPT_RUNNING);
                        fr->first = fr->next = kf->first;
                        fr->receiver = fr->self = self;
                        fr->func = LK_O(kf);
                        fr->returnto = NULL;
                        fr->co.proto = LK_O(kf->frame);
                        lk_vm_doevalfunc(vm);
                        CLROPT(kf->cf.opts, LK_FUNCOPT_RUNNING);
                        v = lk_frame_stackpeek(fr);
                        if(v == NULL) v = vm->t_unknown;
                        slotv = LK_SLOTV(PT_SETITEM_VALUEPTR(
                        pt_set_get(self->co.slots, k)));
                    }
                }
            }
            */
        }
        lk_object_addref(self, v);
        if(slotv->opts & LK_SLOTVOPT_CFIELD) {
            if(v == vm->t_unknown) v = NULL;
            *(lk_object_t **)((ptrdiff_t)self + slotv->v.offset) = v;
        } else {
            slotv->v.value = v;
        }
    } else {
        printf("type mismatch!\n");
        exit(EXIT_FAILURE);
    }
}
int lk_object_calcancestors(lk_object_t *self) {
    lk_object_t *cand = NULL;
    pt_list_t *pars;
    pt_list_t *il, *newancs = pt_list_allocptr(), *ol = pt_list_allocptr();
    int candi, j, k, olc, ilc;
    il = pt_list_allocptr();
    pt_list_pushptr(il, self);
    pt_list_pushptr(ol, il);
    if(LK_OBJECT_HASPARENTS(self)) {
        pars = LK_OBJECT_PARENTS(self);
        for(candi = 0; candi < PT_LIST_COUNT(pars); candi ++) {
            cand = PT_LIST_ATPTR(pars, candi);
            il = pt_list_allocptr();
            if(cand->co.ancestors == NULL) lk_object_calcancestors(cand);
            if(cand->co.ancestors == NULL) return 0;
            pt_list_concat(il, cand->co.ancestors);
            pt_list_pushptr(ol, il);
        }
    } else {
        cand = self->co.proto;
        if(cand != NULL) {
            il = pt_list_allocptr();
            if(cand->co.ancestors == NULL) lk_object_calcancestors(cand);
            if(cand->co.ancestors == NULL) return 0;
            pt_list_concat(il, cand->co.ancestors);
            pt_list_pushptr(ol, il);
        }
    }
    if(LK_OBJECT_HASPARENTS(self)) {
        il = pt_list_allocptr();
        pt_list_concat(il, LK_OBJECT_PARENTS(self));
        pt_list_pushptr(ol, il);
    } else {
        if(self->co.proto != NULL) {
            il = pt_list_allocptr();
            pt_list_pushptr(il, self->co.proto);
            pt_list_pushptr(ol, il);
        }
    }
    startover:
    for(candi = 0, olc = PT_LIST_COUNT(ol); candi < olc; candi ++) {
        il = PT_LIST_ATPTR(ol, candi);
        if(PT_LIST_COUNT(il) < 1) continue;
        cand = PT_LIST_ATPTR(il, 0);
        for(j = 0; j < olc; j ++) {
            il = PT_LIST_ATPTR(ol, j);
            for(k = 1, ilc = PT_LIST_COUNT(il); k < ilc; k ++) {
                if(cand == PT_LIST_ATPTR(il, k)) {
                    cand = NULL;
                    goto nextcandidate;
                }
            }
        }
        pt_list_pushptr(newancs, cand);
        for(j = 0; j < olc; j ++) {
            il = PT_LIST_ATPTR(ol, j);
            for(k = 0, ilc = PT_LIST_COUNT(il); k < ilc; k ++) {
                if(cand == pt_list_getptr(il, k)) pt_list_removeptr(il, k --);
            }
        }
        goto startover;
        nextcandidate:;
    }
    for(j = 0; j < PT_LIST_COUNT(ol); j ++) pt_list_free(PT_LIST_ATPTR(ol, j));
    pt_list_free(ol);
    if(cand == NULL) return 0;
    else {
        pt_list_t *oldancs = self->co.ancestors;
        if(oldancs != NULL) pt_list_free(oldancs);
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
        pt_list_t *ancs = self->co.ancestors; \
        int i, c = PT_LIST_COUNT(ancs); \
        for(i = 1; i < c; i ++) { \
            self = PT_LIST_ATPTR(ancs, i); \
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
struct lk_slotv *lk_object_getdef(lk_object_t *self, lk_object_t *k) {
    pt_set_t *slots;
    pt_setitem_t *si;
    FIND(NULL,
        if((slots = self->co.slots) != NULL
        && (si = pt_set_get(slots, k)) != NULL
        ) return LK_SLOTV(PT_SETITEM_VALUEPTR(si));
    );
}
lk_object_t *lk_object_getslotv(lk_object_t *self, struct lk_slotv *slotv) {
    if(slotv->opts & LK_SLOTVOPT_CFIELD) {
        lk_object_t *v = *(lk_object_t **)((ptrdiff_t)self + slotv->v.offset);
        return v != NULL ? v : LK_VM(self)->t_unknown;
    } else {
        return slotv->v.value;
    }
}
lk_object_t *lk_object_slotget(lk_object_t *self, lk_object_t *k) {
    pt_set_t *slots = self->co.slots;
    pt_setitem_t *si;
    if(slots == NULL) return NULL;
    if((si = pt_set_get(slots, k)) == NULL) return NULL;
    return lk_object_getslotv(self, LK_SLOTV(PT_SETITEM_VALUEPTR(si)));
}
int lk_object_hashcode(const void *k, int capa) {
    return pt_list_hc(LIST(k)) % capa;
}
int lk_object_keycmp(const void *self, const void *other) {
    if(self == other) return 0;
    return !PT_LIST_EQ(LIST(self), LIST(other));
}
