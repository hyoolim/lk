#include "scope.h"
#include "char.h"
#include "ext.h"
#include "number.h"
#include "gc.h"
#include "list.h"
#include "parser.h"
#include "string.h"
#define SCOPE (LK_SCOPE(self))
#define SCOPESTACK (&SCOPE->stack)

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark__scope) {
    if(LIST_ISINIT(SCOPESTACK)) LIST_EACHPTR(SCOPESTACK, i, v, mark(v));
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
static LK_OBJ_DEFFREEFUNC(free__scope) {
    if(LIST_ISINIT(SCOPESTACK)) darray_fin(SCOPESTACK);
}
LK_LIB_DEFINEINIT(lk_scope_libPreInit) {
    vm->t_scope = lk_object_allocwithsize(vm->t_obj, sizeof(lk_scope_t));
    lk_object_setmarkfunc(vm->t_scope, mark__scope);
    lk_object_setfreefunc(vm->t_scope, free__scope);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(Dargs__scope) {
    if(!LIST_ISINIT(SCOPESTACK)) RETURN(lk_list_new(VM));
    else {
        lk_list_t *args = lk_list_newfromlist(VM, SCOPESTACK);
        darray_limit(DARRAY(args), SCOPE->argc);
        RETURN(args);
    }
}
LK_LIB_DEFINECFUNC(DassignB__scope_str_obj) {
    lk_string_t *k = LK_STRING(ARG(0));
    lk_object_t *v = ARG(1);
    struct lk_slot *slot = lk_object_getslotfromany(self, LK_OBJ(k));
    lk_object_t *oldval;
    if(slot == NULL) {
        lk_vm_raisecstr(VM, "Cannot assign to undefined slot %s", k);
    } else if(LK_SLOT_CHECKOPTION(slot, LK_SLOTOPTION_READONLY)) {
        lk_vm_raisecstr(VM, "Cannot assign to readonly slot %s", k);
    } else {
        oldval = lk_object_getvaluefromslot(self, slot);
        if(LK_OBJ_ISFUNC(oldval)
        ) v = LK_OBJ(lk_func_combine(LK_FUNC(oldval), LK_FUNC(v)));
        lk_object_setvalueonslot(self, slot, v);
        v = lk_object_getvaluefromslot(self, slot);
        if(LK_OBJ_ISFUNC(v)) {
            LK_SLOT_SETOPTION(slot, LK_SLOTOPTION_AUTOSEND);
            SETOPT(LK_FUNC(v)->cf.opts, LK_FUNCOASSIGNED);
        }
        RETURN(v);
    }
}
LK_LIB_DEFINECFUNC(include__scope_str_str) {
    lk_scope_t *fr = lk_vm_evalfile(VM,
    darray_toCString(DARRAY(ARG(0))), darray_toCString(DARRAY(ARG(1))));
    if(fr != NULL) {
        qphash_t *from = fr->o.slots;
        if(from != NULL) {
            qphash_t *to = self->o.slots;
            if(to == NULL) to = self->o.slots = qphash_alloc(
            sizeof(struct lk_slot), lk_object_hashcode, lk_object_keycmp);
            SET_EACH(from, i,
                *LK_SLOT(qphash_set(to, i->key)) = *LK_SLOT(SETITEM_VALUEPTR(i));
            );
        }
    }
    RETURN(self);
}
LK_LIB_DEFINECFUNC(raise__scope_err) {
    lk_vm_raiseerror(VM, LK_ERR(ARG(0)));
}
LK_LIB_DEFINECFUNC(raise__scope_str) {
    lk_error_t *err = lk_error_new(VM, VM->t_error, NULL);
    err->text = LK_STRING(ARG(0));
    lk_vm_raiseerror(VM, err);
}
LK_LIB_DEFINECFUNC(redo__scope) {
    if(LIST_ISINIT(SCOPESTACK)) darray_clear(SCOPESTACK);
    SCOPE->next = SCOPE->first;
    DONE;
}
LK_LIB_DEFINECFUNC(require__scope_str_str) {
    RETURN(lk_vm_evalfile(VM,
    darray_toCString(DARRAY(ARG(0))), darray_toCString(DARRAY(ARG(1)))));
}
LK_LIB_DEFINECFUNC(rescue__scope_f) {
    RETURN(lk_object_getvaluefromslot(self, lk_object_setslot(
    self, LK_OBJ(VM->str_rescue), VM->t_func, ARG(0))));
}
LK_LIB_DEFINECFUNC(RESOURCE__scope) {
    RETURN(VM->rsrc != NULL && !VM->rsrc->isstring
    ? LK_OBJ(VM->rsrc->rsrc) : NIL);
}
LK_LIB_DEFINECFUNC(retry__scope) {
    lk_scope_t *caller = SCOPE->caller;
    lk_instr_t *i = caller->current->prev;
    SCOPE->next = NULL;
    SCOPE->returnto = caller;
    while(i->prev != NULL && !(i->prev->opts & LK_INSTROEND)) i = i->prev;
    caller->next = i;
    DONE;
}
LK_LIB_DEFINECFUNC(return__scope) {
    lk_scope_t *f = SCOPE;
    for(; ; f = LK_OBJ_PROTO(f)) {
        if(f == NULL) lk_vm_abort(VM, NULL);
        if(CHKOPT(LK_FUNC(f->func)->cf.opts, LK_FUNCOASSIGNED)) break;
    }
    f = f->returnto;
    SCOPE->next = NULL;
    SCOPE->returnto = f;
    if(LIST_ISINIT(&local->stack)) {
        if(!LIST_ISINIT(SCOPESTACK)) darray_initptr(SCOPESTACK);
        darray_concat(SCOPESTACK, &local->stack);
    }
    DONE;
}
LK_LIB_DEFINEINIT(lk_scope_libInit) {
    lk_object_t *scope = vm->t_scope, *obj = vm->t_obj, *instr = vm->t_instr,
                *str = vm->t_string, *err = vm->t_error, *f = vm->t_func;
    lk_lib_setGlobal("Scope", scope);
    lk_lib_setCFunc(scope, ".args", Dargs__scope, NULL);
    lk_lib_setCFunc(scope, "=", DassignB__scope_str_obj, str, obj, NULL);
    lk_lib_setCField(scope, ".caller", scope, offsetof(lk_scope_t, caller));
    lk_lib_setCField(scope, ".current", instr, offsetof(lk_scope_t, current));
    lk_lib_setCField(scope, ".first", instr, offsetof(lk_scope_t, first));
    lk_lib_setCField(scope, ".scope", obj, offsetof(lk_scope_t, scope));
    lk_lib_setCField(scope, ".function", obj, offsetof(lk_scope_t, func));
    lk_lib_setCField(scope, ".next", instr, offsetof(lk_scope_t, next));
    lk_lib_setCField(scope, ".receiver", obj, offsetof(lk_scope_t, receiver));
    lk_lib_setCField(scope, ".return_to", scope, offsetof(lk_scope_t, returnto));
    lk_lib_setCFunc(scope, "include", include__scope_str_str, str, str, NULL);
    lk_lib_setCFunc(scope, "raise", raise__scope_err, err, NULL);
    lk_lib_setCFunc(scope, "raise", raise__scope_str, str, NULL);
    lk_lib_setCField(scope, "receiver", obj, offsetof(lk_scope_t, receiver));
    lk_lib_setCFunc(scope, "redo", redo__scope, NULL);
    lk_lib_setCFunc(scope, "require", require__scope_str_str, str, str, NULL);
    lk_lib_setCFunc(scope, "rescue", rescue__scope_f, f, NULL);
    lk_lib_setCFunc(scope, "RESOURCE", RESOURCE__scope, NULL);
    lk_lib_setCFunc(scope, "retry", retry__scope, NULL);
    lk_lib_setCFunc(scope, "return", return__scope, -1);
}

/* create a new scope based on the current one set in vm */
lk_scope_t *lk_scope_new(lk_vm_t *vm) {
    lk_scope_t *parent = vm->currentScope;
    lk_scope_t *self;

    /* optimization to reduce the number of scopes created */
    if(parent->child != NULL && parent->child->o.mark.isref == 0) {
        vm->stat.recycledScopes ++;
        self = parent->child;
        self->o.parent = LK_OBJ(parent);
        darray_clear(&self->stack);
        if(self->o.slots != NULL) {
            qphash_clear(self->o.slots);
        }
    } else {
        self = parent->child = LK_SCOPE(lk_object_alloc(LK_OBJ(parent)));
    }

    /* init scope struct */
    vm->stat.totalScopes ++;
    vm->currentScope = self;
    self->type = LK_SCOPETYPE_RETURN;
    self->scope = self;
    self->receiver = LK_OBJ(self);
    self->self = parent != NULL ? parent->self : NULL;
    self->caller = self->returnto = parent;
    return self;
}
void lk_scope_stackpush(lk_scope_t *self, lk_object_t *v) {
    assert(v != NULL);
    if(!LIST_ISINIT(&self->stack)) darray_initptr(&self->stack);
    darray_pushptr(&self->stack, lk_object_addref(LK_OBJ(self), v));
}

/* update */
lk_object_t *lk_scope_stackpop(lk_scope_t *self) {
    assert(LIST_ISINIT(&self->stack));
    assert(LIST_COUNT(&self->stack) > 0);
    return darray_popptr(&self->stack);
}
lk_object_t *lk_scope_stackpeek(lk_scope_t *self) {
    lk_vm_t *vm = LK_VM(self);
    if(!LIST_ISINIT(&self->stack)) return vm->t_nil;
    if(LIST_COUNT(&self->stack) < 1) return vm->t_nil;
    return darray_peekptr(&self->stack);
}
lk_list_t *lk_scope_stacktolist(lk_scope_t *self) {
    lk_list_t *stack = lk_list_new(LK_VM(self));
    if(LIST_ISINIT(&self->stack)) darray_copy(DARRAY(stack), &self->stack);
    return stack;
}
