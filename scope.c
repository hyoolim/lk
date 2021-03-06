#include "scope.h"
#include "char.h"
#include "lib.h"
#include "num.h"
#include "gc.h"
#include "list.h"
#include "parser.h"
#include "str.h"
#define SCOPE (LK_SCOPE(self))
#define SCOPESTACK (&SCOPE->stack)

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark_scope) {
    if(DARRAY_ISINIT(SCOPESTACK)) DARRAY_EACHPTR(SCOPESTACK, i, v, mark(v));
    mark(LK_OBJ(SCOPE->scope));
    mark(LK_OBJ(SCOPE->receiver));
    mark(LK_OBJ(SCOPE->self));
    mark(LK_OBJ(SCOPE->caller));
    mark(LK_OBJ(SCOPE->child));
    mark(LK_OBJ(SCOPE->returnto));
    mark(LK_OBJ(SCOPE->first));
    mark(LK_OBJ(SCOPE->next));
    mark(LK_OBJ(SCOPE->current));
    mark(LK_OBJ(SCOPE->func));
}
static void free_scope(lk_obj_t *self) {
    if(DARRAY_ISINIT(SCOPESTACK)) darray_fin(SCOPESTACK);
}
void lk_scope_typeinit(lk_vm_t *vm) {
    vm->t_scope = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_scope_t));
    lk_obj_setmarkfunc(vm->t_scope, mark_scope);
    lk_obj_setfreefunc(vm->t_scope, free_scope);
}

/* ext map - funcs */
static void Dargs_scope(lk_obj_t *self, lk_scope_t *local) {
    if(!DARRAY_ISINIT(SCOPESTACK)) RETURN(lk_list_new(VM));
    else {
        lk_list_t *args = lk_list_new_fromdarray(VM, SCOPESTACK);
        darray_limit(DARRAY(args), SCOPE->argc);
        RETURN(args);
    }
}
static void DassignB_scope_str_obj(lk_obj_t *self, lk_scope_t *local) {
    lk_str_t *k = LK_STRING(ARG(0));
    lk_obj_t *v = ARG(1);
    struct lk_slot *slot = lk_obj_getslotfromany(self, LK_OBJ(k));
    lk_obj_t *oldval;
    if(slot == NULL) {
        lk_vm_raisecstr(VM, "Cannot assign to undefined slot %s", k);
    } else if(LK_SLOT_CHECKOPTION(slot, LK_SLOTOPTION_READONLY)) {
        lk_vm_raisecstr(VM, "Cannot assign to readonly slot %s", k);
    } else {
        oldval = lk_obj_getvaluefromslot(self, slot);
        if(LK_OBJ_ISFUNC(oldval)
        ) v = LK_OBJ(lk_func_combine(LK_FUNC(oldval), LK_FUNC(v)));
        lk_obj_setvalueonslot(self, slot, v);
        v = lk_obj_getvaluefromslot(self, slot);
        if(LK_OBJ_ISFUNC(v)) {
            LK_SLOT_SETOPTION(slot, LK_SLOTOPTION_AUTOSEND);
            SETOPT(LK_FUNC(v)->cf.opts, LK_FUNCOASSIGNED);
        }
        RETURN(v);
    }
}
static void include_scope_str_str(lk_obj_t *self, lk_scope_t *local) {
    lk_scope_t *fr = lk_vm_evalfile(VM,
    darray_str_tocstr(DARRAY(ARG(0))), darray_str_tocstr(DARRAY(ARG(1))));
    if(fr != NULL) {
        qphash_t *from = fr->o.slots;
        if(from != NULL) {
            qphash_t *to = self->o.slots;
            if(to == NULL) to = self->o.slots = qphash_alloc(
            sizeof(struct lk_slot), lk_obj_hashcode, lk_obj_keycmp);
            SET_EACH(from, i,
                *LK_SLOT(qphash_set(to, i->key)) = *LK_SLOT(SETITEM_VALUEPTR(i));
            );
        }
    }
    RETURN(self);
}
static void raise_scope_err(lk_obj_t *self, lk_scope_t *local) {
    lk_vm_raiseerr(VM, LK_ERROR(ARG(0)));
}
static void raise_scope_str(lk_obj_t *self, lk_scope_t *local) {
    lk_err_t *err = lk_err_new(VM, VM->t_err, NULL);
    err->message = LK_STRING(ARG(0));
    lk_vm_raiseerr(VM, err);
}
static void redo_scope(lk_obj_t *self, lk_scope_t *local) {
    if(DARRAY_ISINIT(SCOPESTACK)) darray_clear(SCOPESTACK);
    SCOPE->next = SCOPE->first;
    DONE;
}
static void require_scope_str_str(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_vm_evalfile(VM,
    darray_str_tocstr(DARRAY(ARG(0))), darray_str_tocstr(DARRAY(ARG(1)))));
}
static void rescue_scope_f(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_obj_getvaluefromslot(self, lk_obj_setslot(
    self, LK_OBJ(VM->str_rescue), VM->t_func, ARG(0))));
}
static void RESOURCE_scope(lk_obj_t *self, lk_scope_t *local) {
    RETURN(VM->rsrc != NULL && !VM->rsrc->isstr
    ? LK_OBJ(VM->rsrc->rsrc) : NIL);
}
static void retry_scope(lk_obj_t *self, lk_scope_t *local) {
    lk_scope_t *caller = SCOPE->caller;
    lk_instr_t *i = caller->current->prev;
    SCOPE->next = NULL;
    SCOPE->returnto = caller;
    while(i->prev != NULL && !(i->prev->opts & LK_INSTROEND)) i = i->prev;
    caller->next = i;
    DONE;
}
static void return_scope(lk_obj_t *self, lk_scope_t *local) {
    lk_scope_t *f = SCOPE;
    for(; ; f = LK_OBJ_PROTO(f)) {
        if(f == NULL) lk_vm_abort(VM, NULL);
        if(CHKOPT(LK_FUNC(f->func)->cf.opts, LK_FUNCOASSIGNED)) break;
    }
    f = f->returnto;
    SCOPE->next = NULL;
    SCOPE->returnto = f;
    if(DARRAY_ISINIT(&local->stack)) {
        if(!DARRAY_ISINIT(SCOPESTACK)) darray_ptr_init(SCOPESTACK);
        darray_concat(SCOPESTACK, &local->stack);
    }
    DONE;
}
void lk_scope_libinit(lk_vm_t *vm) {
    lk_obj_t *scope = vm->t_scope, *obj = vm->t_obj, *instr = vm->t_instr,
                *str = vm->t_str, *err = vm->t_err, *f = vm->t_func;
    lk_global_set("Scope", scope);
    lk_obj_set_cfunc_lk(scope, ".args", Dargs_scope, NULL);
    lk_obj_set_cfunc_lk(scope, "=", DassignB_scope_str_obj, str, obj, NULL);
    lk_obj_set_cfield(scope, ".caller", scope, offsetof(lk_scope_t, caller));
    lk_obj_set_cfield(scope, ".current", instr, offsetof(lk_scope_t, current));
    lk_obj_set_cfield(scope, ".first", instr, offsetof(lk_scope_t, first));
    lk_obj_set_cfield(scope, ".scope", obj, offsetof(lk_scope_t, scope));
    lk_obj_set_cfield(scope, ".function", obj, offsetof(lk_scope_t, func));
    lk_obj_set_cfield(scope, ".next", instr, offsetof(lk_scope_t, next));
    lk_obj_set_cfield(scope, ".receiver", obj, offsetof(lk_scope_t, receiver));
    lk_obj_set_cfield(scope, ".return_to", scope, offsetof(lk_scope_t, returnto));
    lk_obj_set_cfunc_lk(scope, "include", include_scope_str_str, str, str, NULL);
    lk_obj_set_cfunc_lk(scope, "raise", raise_scope_err, err, NULL);
    lk_obj_set_cfunc_lk(scope, "raise", raise_scope_str, str, NULL);
    lk_obj_set_cfield(scope, "receiver", obj, offsetof(lk_scope_t, receiver));
    lk_obj_set_cfunc_lk(scope, "redo", redo_scope, NULL);
    lk_obj_set_cfunc_lk(scope, "require", require_scope_str_str, str, str, NULL);
    lk_obj_set_cfunc_lk(scope, "rescue", rescue_scope_f, f, NULL);
    lk_obj_set_cfunc_lk(scope, "RESOURCE", RESOURCE_scope, NULL);
    lk_obj_set_cfunc_lk(scope, "retry", retry_scope, NULL);
    lk_obj_set_cfunc_lk(scope, "return", return_scope, -1);
}

/* create a new scope based on the current one set in vm */
lk_scope_t *lk_scope_new(lk_vm_t *vm) {
    lk_scope_t *parent = vm->currscope;
    lk_scope_t *self;

    /* optimization to reduce the num of scopes created */
    if(parent->child != NULL && parent->child->o.mark.isref == 0) {
        vm->stat.recycledscopes ++;
        self = parent->child;
        self->o.parent = LK_OBJ(parent);
        darray_clear(&self->stack);
        if(self->o.slots != NULL) {
            qphash_clear(self->o.slots);
        }
    } else {
        self = parent->child = LK_SCOPE(lk_obj_alloc(LK_OBJ(parent)));
    }

    /* init scope struct */
    vm->stat.totalscopes ++;
    vm->currscope = self;
    self->type = LK_SCOPETYPE_RETURN;
    self->scope = self;
    self->receiver = LK_OBJ(self);
    self->self = parent != NULL ? parent->self : NULL;
    self->caller = self->returnto = parent;
    return self;
}
void lk_scope_stackpush(lk_scope_t *self, lk_obj_t *v) {
    assert(v != NULL);
    if(!DARRAY_ISINIT(&self->stack)) darray_ptr_init(&self->stack);
    darray_ptr_push(&self->stack, lk_obj_addref(LK_OBJ(self), v));
}

/* update */
lk_obj_t *lk_scope_stackpop(lk_scope_t *self) {
    assert(DARRAY_ISINIT(&self->stack));
    assert(DARRAY_COUNT(&self->stack) > 0);
    return darray_ptr_pop(&self->stack);
}
lk_obj_t *lk_scope_stackpeek(lk_scope_t *self) {
    lk_vm_t *vm = LK_VM(self);
    if(!DARRAY_ISINIT(&self->stack)) return vm->t_nil;
    if(DARRAY_COUNT(&self->stack) < 1) return vm->t_nil;
    return darray_ptr_peek(&self->stack);
}
lk_list_t *lk_scope_stacktolist(lk_scope_t *self) {
    lk_list_t *stack = lk_list_new(LK_VM(self));
    if(DARRAY_ISINIT(&self->stack)) darray_copy(DARRAY(stack), &self->stack);
    return stack;
}
