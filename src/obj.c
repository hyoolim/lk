#include "lib.h"
#include <stdarg.h>

// pointer tagging: lowest bit of parent ptr used as multi-parent flag
_Static_assert(_Alignof(lk_obj_t) >= 2, "lk_obj_t must be at least 2-byte aligned for pointer tagging");

// ext map - types
void lk_obj_type_init(lk_vm_t *vm) {
    lk_obj_t *o = vm->t_obj = mem_alloc(sizeof(lk_obj_t));
    o->o.tag = mem_alloc(sizeof(lk_tag_t));
    o->o.tag->refc = 1;
    o->o.tag->vm = vm;
    o->o.tag->size = sizeof(lk_obj_t);
    o->o.parent = NULL;
    vec_ptr_push(o->o.ancestors = vec_ptr_alloc(), o);
}

// ext map - funcs
static void Ddefine_and_assignB_obj_str_obj_obj(lk_obj_t *self, lk_scope_t *local);

static void Ddefine_obj_str_obj(lk_obj_t *self, lk_scope_t *local) {
    local->argc++;
    vec_ptr_push(&local->stack, NIL);
    GOTO(Ddefine_and_assignB_obj_str_obj_obj);
}

static void Ddefine_and_assignB_obj_str_obj(lk_obj_t *self, lk_scope_t *local) {
    local->argc++;
    vec_ptr_insert(&local->stack, 1, VM->t_obj);
    GOTO(Ddefine_and_assignB_obj_str_obj_obj);
}

static void Ddefine_and_assignB_obj_str_obj_obj(lk_obj_t *self, lk_scope_t *local) {
    lk_obj_t *k = ARG(0);
    lk_obj_t *v = ARG(2);
    struct lk_slot *slot;

    if (self->o.slots != NULL && qphash_get(self->o.slots, k) != NULL) {
        lk_vm_raise_cstr(VM, "Cannot redefine %s", k);
    }
    slot = lk_obj_setslot(self, k, ARG(1), v);
    v = lk_obj_get_value_from_slot(self, slot);

    if (LK_OBJ_ISFUNC(v)) {
        LK_SLOT_SETOPTION(slot, LK_SLOTOPTION_AUTOSEND);
        SETOPT(LK_FUNC(v)->cf.opts, LK_FUNCOASSIGNED);
        LK_FUNC(v)->cf.doc = local->caller->current->prev->comment;
    }
    RETURN(v);
}

static void Did_obj(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, (double)(uintptr_t)self));
}

static void Dretrieve_obj_str(lk_obj_t *self, lk_scope_t *local) {
    struct lk_slot *slot = lk_obj_get_slot_from_any(self, ARG(0));

    if (slot != NULL)
        RETURN(lk_obj_get_value_from_slot(self, slot));
    else
        RETURN(NIL);
}

static void Dself_obj(lk_obj_t *self, lk_scope_t *local) {
    RETURN(self);
}

static void Dslots_obj(lk_obj_t *self, lk_scope_t *local) {
    lk_list_t *slots = lk_list_new(VM);

    if (self->o.slots != NULL) {
        SET_EACH(self->o.slots, i, vec_ptr_push(VEC(slots), (void *)i->key););
    }
    RETURN(slots);
}

static void alloc_obj(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_obj_alloc(self));
}

static void also_obj_obj(lk_obj_t *self, lk_scope_t *local) {
    lk_obj_extend(self, ARG(0));
    RETURN(self);
}

static void ancestor_obj_obj(lk_obj_t *self, lk_scope_t *local) {
    RETURN(LK_OBJ_ISTYPE(self, ARG(0)) ? VM->t_true : VM->t_false);
}

static void ancestors_obj(lk_obj_t *self, lk_scope_t *local) {
    if (self->o.ancestors != NULL || lk_obj_calc_ancestors(self)) {
        RETURN(lk_list_new_from_darray(VM, self->o.ancestors));

    } else {
        printf("BUG: Throw proper ancestor err here\n");
        exit(EXIT_FAILURE);
    }
}

static void clone_obj(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_obj_clone(self));
}

static void do_obj_f(lk_obj_t *self, lk_scope_t *local) {
    lk_lfunc_t *lf = LK_LFUNC(ARG(0));
    lk_scope_t *fr = lk_scope_new(VM);
    fr->first = fr->next = lf->first;
    fr->receiver = fr->self = self;
    fr->func = LK_OBJ(lf);
    fr->returnto = NULL;
    fr->o.parent = LK_OBJ(lf->scope);
    lk_vm_do_eval_func(VM);
    RETURN(self);
}

static void import_obj_obj(lk_obj_t *self, lk_scope_t *local) {
    qphash_t *from = ARG(0)->o.slots;

    if (from != NULL) {
        qphash_t *to = self->o.slots;

        if (to == NULL)
            to = self->o.slots = qphash_alloc(sizeof(struct lk_slot), lk_obj_hash_code, lk_obj_key_cmp);
        SET_EACH(from, i, *LK_SLOT(qphash_set(to, i->key)) = *LK_SLOT(SETITEM_VALUEPTR(i)););
    }
    RETURN(self);
}

static void parents_obj(lk_obj_t *self, lk_scope_t *local) {
    if (LK_OBJ_HASPARENTS(self)) {
        RETURN(lk_list_new_from_darray(VM, LK_OBJ_PARENTS(self)));

    } else {
        lk_list_t *ret = lk_list_new(VM);
        vec_ptr_push(VEC(ret), self->o.parent);
        RETURN(ret);
    }
}

static void parent_obj(lk_obj_t *self, lk_scope_t *local) {
    if (LK_OBJ_HASPARENTS(self)) {
        vec_t *pars = LK_OBJ_PARENTS(self);
        RETURN(VEC_COUNT(pars) > 0 ? vec_ptr_get(pars, -1) : NIL);
    }
    RETURN(self->o.parent);
}

static void with_obj_f(lk_obj_t *self, lk_scope_t *local) {
    do_obj_f(lk_obj_add_ref(LK_OBJ(local), lk_obj_alloc(self)), local);
}

void lk_obj_lib_init(lk_vm_t *vm) {
    lk_obj_t *obj = vm->t_obj, *str = vm->t_str, *f = vm->t_func;
    lk_global_set("Object", obj);
    lk_obj_set_cfunc_lk(obj, ".", Dself_obj, NULL);
    lk_obj_set_cfunc_lk(obj, ":", Ddefine_obj_str_obj, str, obj, NULL);
    lk_obj_set_cfunc_lk(obj, ":=", Ddefine_and_assignB_obj_str_obj, str, obj, NULL);
    lk_obj_set_cfunc_lk(obj, ":=", Ddefine_and_assignB_obj_str_obj_obj, str, obj, obj, NULL);
    lk_obj_set_cfunc_lk(obj, "_id", Did_obj, NULL);
    lk_obj_set_cfunc_lk(obj, "_retrieve", Dretrieve_obj_str, str, NULL);
    lk_obj_set_cfunc_lk(obj, "_self", Dself_obj, NULL);
    lk_obj_set_cfunc_lk(obj, "_slots", Dslots_obj, NULL);
    lk_obj_set_cfunc_lk(obj, "alloc", alloc_obj, NULL);
    lk_obj_set_cfunc_lk(obj, "also", also_obj_obj, obj, NULL);
    lk_obj_set_cfunc_lk(obj, "ancestor?", ancestor_obj_obj, obj, NULL);
    lk_obj_set_cfunc_lk(obj, "ancestors", ancestors_obj, NULL);
    lk_obj_set_cfunc_lk(obj, "clone", clone_obj, NULL);
    lk_obj_set_cfunc_lk(obj, "do", do_obj_f, f, NULL);
    lk_obj_set_cfunc_lk(obj, "extend", do_obj_f, f, NULL);
    lk_obj_set_cfunc_lk(obj, "import", import_obj_obj, obj, NULL);
    lk_obj_set_cfunc_lk(obj, "is?", ancestor_obj_obj, obj, NULL);
    lk_obj_set_cfunc_lk(obj, "parents", parents_obj, NULL);
    lk_obj_set_cfunc_lk(obj, "parent", parent_obj, NULL);
    lk_obj_set_cfunc_lk(obj, "with", with_obj_f, f, NULL);
}

// new
static lk_tag_t *tag_clone(lk_tag_t *self) {
    lk_tag_t *clone = mem_alloc(sizeof(lk_tag_t));
    memcpy(clone, self, sizeof(lk_tag_t));
    clone->refc = 1;
    return clone;
}

lk_obj_t *lk_obj_alloc_with_size(lk_obj_t *parent, size_t s) {
    lk_gc_t *gc = LK_VM(parent)->gc;
    lk_obj_t *self = mem_alloc(s);
    lk_tag_t *tag = parent->o.tag;

    if (tag->size == s)
        tag->refc++;
    else
        (tag = tag_clone(tag))->size = s;
    self->o.parent = parent;
    self->o.tag = tag;

    if (tag->alloc_func != NULL)
        tag->alloc_func(self, parent);

    if (gc != NULL) {
        gc->newvalues++;
        lk_objgroup_insert(gc->unused, self);
    }
    return self;
}

lk_obj_t *lk_obj_alloc(lk_obj_t *parent) {
    return lk_obj_alloc_with_size(parent, parent->o.tag->size);
}

lk_obj_t *lk_obj_clone(lk_obj_t *self) {
    lk_obj_t *c = lk_obj_alloc(LK_OBJ_PROTO(self));

    if (c->o.tag->alloc_func != NULL)
        c->o.tag->alloc_func(c, self);
    return c;
}

void lk_obj_just_free(lk_obj_t *self) {
    lk_tag_t *tag = self->o.tag;

    if (tag->free_func != NULL)
        tag->free_func(self);
    if (LK_OBJ_HASPARENTS(self))
        vec_free(LK_OBJ_PARENTS(self));
    if (self->o.ancestors != NULL)
        vec_free(self->o.ancestors);
    if (self->o.slots != NULL)
        qphash_free(self->o.slots);
    if (--tag->refc < 1)
        mem_free(tag);
    mem_free(self);
}

void lk_obj_free(lk_obj_t *self) {
    lk_objgroup_remove(self);
    lk_obj_just_free(self);
}

// update - tag
#define LK_OBJ_IMPLTAGSETTER(t, field) \
    LK_OBJ_DEFTAGSETTER(t, field) { \
        lk_tag_t *tag = self->o.tag; \
        if (tag->field != field) { \
            if (tag->refc > 1) { \
                tag->refc--; \
                tag = self->o.tag = tag_clone(tag); \
            } \
            tag->field = field; \
        } \
    } \
    LK_OBJ_DEFTAGSETTER(t, field)

LK_OBJ_IMPLTAGSETTER(lk_tagallocfunc_t *, alloc_func);
LK_OBJ_IMPLTAGSETTER(lk_tagmarkfunc_t *, mark_func);
LK_OBJ_IMPLTAGSETTER(lk_tagfreefunc_t *, free_func);

// update
void lk_obj_extend(lk_obj_t *self, lk_obj_t *parent) {
    vec_t *parents;

    if (LK_OBJ_HASPARENTS(self)) {
        parents = LK_OBJ_PARENTS(self);

    } else {
        parents = vec_ptr_alloc();
        vec_ptr_push(parents, self->o.parent);
        self->o.parent = LK_OBJ((ptrdiff_t)parents | 1); // lowest bit = multi-parent flag; requires 2-byte alignment
    }
    vec_ptr_unshift(parents, parent);

    if (!lk_obj_calc_ancestors(self)) {
        printf("BUG: Throw proper ancestor err here\n");
        exit(EXIT_FAILURE);
    }
}

struct lk_slot *lk_obj_setslot(lk_obj_t *self, lk_obj_t *k, lk_obj_t *check, lk_obj_t *v) {
    struct lk_slot *slot = lk_obj_getslot(self, k);

    if (slot == NULL) {
        uint32_t first = vec_str_get(VEC(k), 0);

        if (self->o.slots == NULL) {
            self->o.slots = qphash_alloc(sizeof(struct lk_slot), lk_obj_hash_code, lk_obj_key_cmp);
        }
        slot = LK_SLOT(qphash_set(self->o.slots, k));
        slot->check = check;

        if ('A' <= first && first <= 'Z') {
            LK_SLOT_SETOPTION(slot, LK_SLOTOPTION_READONLY);
            lk_obj_setslot(v, LK_OBJ(LK_VM(self)->str_type), LK_OBJ(LK_VM(self)->t_str), LK_OBJ(k));
        }
    }
    lk_obj_set_value_on_slot(self, slot, v);
    return slot;
}

struct lk_slot *lk_obj_set_slot_by_cstr(lk_obj_t *self, const char *k, lk_obj_t *check, lk_obj_t *v) {
    return lk_obj_setslot(self, LK_OBJ(lk_str_new_from_cstr(LK_VM(self), k)), check, v);
}

void lk_obj_set_value_on_slot(lk_obj_t *self, struct lk_slot *slot, lk_obj_t *v) {
    lk_vm_t *vm = LK_VM(self);

    if (v == NULL)
        v = vm->t_nil;

    if (v == vm->t_nil || LK_OBJ_ISTYPE(v, slot->check)) {
        lk_obj_add_ref(self, v);

        if (LK_SLOT_GETTYPE(slot) == LK_SLOTTYPE_CFIELDLKOBJ) {
            if (v == vm->t_nil)
                v = NULL;
            *(lk_obj_t **)((ptrdiff_t)self + slot->value.coffset) = v;

        } else {
            slot->value.lkobj = v;
        }

    } else {
        printf("type mismatch!\n");
        exit(EXIT_FAILURE);
    }
}

int lk_obj_calc_ancestors(lk_obj_t *self) {
    lk_obj_t *cand = NULL;
    vec_t *pars;
    vec_t *il, *newancs = vec_ptr_alloc(), *ol = vec_ptr_alloc();
    int candi, j, k, olc, ilc;

    il = vec_ptr_alloc();
    vec_ptr_push(il, self);
    vec_ptr_push(ol, il);

    if (LK_OBJ_HASPARENTS(self)) {
        pars = LK_OBJ_PARENTS(self);

        for (candi = 0; candi < VEC_COUNT(pars); candi++) {
            cand = VEC_ATPTR(pars, candi);
            il = vec_ptr_alloc();

            if (cand->o.ancestors == NULL)
                lk_obj_calc_ancestors(cand);
            if (cand->o.ancestors == NULL)
                return 0;
            vec_concat(il, cand->o.ancestors);
            vec_ptr_push(ol, il);
        }

    } else {
        cand = self->o.parent;

        if (cand != NULL) {
            il = vec_ptr_alloc();

            if (cand->o.ancestors == NULL)
                lk_obj_calc_ancestors(cand);
            if (cand->o.ancestors == NULL)
                return 0;
            vec_concat(il, cand->o.ancestors);
            vec_ptr_push(ol, il);
        }
    }

    if (LK_OBJ_HASPARENTS(self)) {
        il = vec_ptr_alloc();
        vec_concat(il, LK_OBJ_PARENTS(self));
        vec_ptr_push(ol, il);

    } else {
        if (self->o.parent != NULL) {
            il = vec_ptr_alloc();
            vec_ptr_push(il, self->o.parent);
            vec_ptr_push(ol, il);
        }
    }

startover:
    for (candi = 0, olc = VEC_COUNT(ol); candi < olc; candi++) {
        il = VEC_ATPTR(ol, candi);

        if (VEC_COUNT(il) < 1)
            continue;
        cand = VEC_ATPTR(il, 0);

        for (j = 0; j < olc; j++) {
            il = VEC_ATPTR(ol, j);

            for (k = 1, ilc = VEC_COUNT(il); k < ilc; k++) {
                if (cand == VEC_ATPTR(il, k)) {
                    cand = NULL;
                    goto nextcandidate;
                }
            }
        }

        vec_ptr_push(newancs, cand);

        for (j = 0; j < olc; j++) {
            il = VEC_ATPTR(ol, j);

            for (k = 0, ilc = VEC_COUNT(il); k < ilc; k++) {
                if (cand == vec_ptr_get(il, k))
                    vec_ptr_remove(il, k--);
            }
        }

        goto startover;
    nextcandidate:;
    }
    for (j = 0; j < VEC_COUNT(ol); j++)
        vec_free(VEC_ATPTR(ol, j));
    vec_free(ol);

    if (cand == NULL)
        return 0;
    else {
        vec_t *oldancs = self->o.ancestors;

        if (oldancs != NULL)
            vec_free(oldancs);
        self->o.ancestors = newancs;
        return 1;
    }
}

static lk_cfunc_t *lk_obj_setcfunc(lk_obj_t *self, const char *name, lk_cfunc_lk_t *cfunc, va_list *args) {
    lk_vm_t *vm = LK_VM(self);
    lk_str_t *nameobj = lk_str_new_from_cstr(vm, name);
    lk_cfunc_t *cfuncobj = lk_cfunc_new(vm, cfunc, 0, 0);
    struct lk_slot *slot = lk_obj_getslot(self, LK_OBJ(nameobj));

    if (slot != NULL) {
        lk_obj_t *old = lk_obj_get_value_from_slot(self, slot);

        if (LK_OBJ_ISFUNC(old)) {
            old = LK_OBJ(lk_func_combine(LK_FUNC(old), LK_FUNC(cfuncobj)));
        }
        lk_obj_set_value_on_slot(self, slot, old);

    } else {
        slot = lk_obj_setslot(self, LK_OBJ(nameobj), vm->t_func, LK_OBJ(cfuncobj));
    }
    LK_SLOT_SETOPTION(slot, LK_SLOTOPTION_AUTOSEND);

    for (int i = 0;; i++) {
        lk_obj_t *arg = va_arg(*args, lk_obj_t *);

        if (arg == NULL) {
            cfuncobj->cf.minargc = cfuncobj->cf.maxargc = i;
            break;
        }
        if (arg == (lk_obj_t *)-1) {
            cfuncobj->cf.maxargc = INT_MAX;
            break;
        }
        vec_ptr_set(cfuncobj->cf.sigs, i, lk_sig_new(vm, NULL, arg));
    }
    return cfuncobj;
}

void lk_obj_set_cfunc_lk(lk_obj_t *self, const char *name, lk_cfunc_lk_t *cfunc, ...) {
    va_list args;
    va_start(args, cfunc);
    lk_obj_setcfunc(self, name, cfunc, &args);
    va_end(args);
}

void lk_obj_set_cfunc_creturn(lk_obj_t *self, const char *name, ...) {
    va_list args;
    va_start(args, name);
    lk_obj_setcfunc(self, name, va_arg(args, lk_cfunc_lk_t *), &args)->cc = LK_CFUNC_CC_CRETURN;
    va_end(args);
}

void lk_obj_set_cfunc_cvoid(lk_obj_t *self, const char *name, ...) {
    va_list args;
    va_start(args, name);
    lk_obj_setcfunc(self, name, va_arg(args, lk_cfunc_lk_t *), &args)->cc = LK_CFUNC_CC_CVOID;
    va_end(args);
}

// info
#define FIND(nil, check) \
    do { \
        while (1) { \
            check if (self->o.ancestors != NULL) goto checkancestors; \
            self = self->o.parent; \
        } \
    checkancestors: { \
        vec_t *ancs = self->o.ancestors; \
        int i, c = VEC_COUNT(ancs); \
        for (i = 1; i < c; i++) { \
            self = VEC_ATPTR(ancs, i); \
            check \
        } \
        return (nil); \
    } \
    } while (0)

int lk_obj_isa(lk_obj_t *self, lk_obj_t *t) {
    int dist = 1;
    FIND(0, if (self == t) return dist; dist++;);
}

struct lk_slot *lk_obj_getslot(lk_obj_t *self, lk_obj_t *k) {
    qphash_t *slots = self->o.slots;
    setitem_t *item;

    if (slots == NULL || (item = qphash_get(slots, k)) == NULL)
        return NULL;
    return LK_SLOT(SETITEM_VALUEPTR(item));
}

struct lk_slot *lk_obj_get_slot_from_any(lk_obj_t *self, lk_obj_t *k) {
    qphash_t *slots;
    setitem_t *si;
    FIND(NULL,
         if ((slots = self->o.slots) != NULL &&
             (si = qphash_get(slots, k)) != NULL) return LK_SLOT(SETITEM_VALUEPTR(si)););
}

lk_obj_t *lk_obj_get_value_from_slot(lk_obj_t *self, struct lk_slot *slot) {
    if (LK_SLOT_GETTYPE(slot) == LK_SLOTTYPE_CFIELDLKOBJ) {
        lk_obj_t *v = *(lk_obj_t **)((ptrdiff_t)self + slot->value.coffset);
        return v != NULL ? v : LK_VM(self)->t_nil;

    } else {
        return slot->value.lkobj;
    }
}

int lk_obj_hash_code(const void *k, int cap) {
    return vec_hc(VEC(k)) % cap;
}

int lk_obj_key_cmp(const void *self, const void *other) {
    if (self == other)
        return 0;
    return !VEC_EQ(VEC(self), VEC(other));
}
