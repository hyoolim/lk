#include "frame.h"
#include "char.h"
#include "ext.h"
#include "fixnum.h"
#include "gc.h"
#include "list.h"
#include "parser.h"
#include "string.h"
#define FRAME (LK_FRAME(self))
#define FRAMESTACK (&FRAME->stack)

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark__fra) {
    if(LIST_ISINIT(FRAMESTACK)) LIST_EACHPTR(FRAMESTACK, i, v, mark(v));
    mark(LK_OBJ(FRAME->frame));
    mark(LK_OBJ(FRAME->receiver));
    mark(LK_OBJ(FRAME->self));
    mark(LK_OBJ(FRAME->caller));
    mark(LK_OBJ(FRAME->child));
    mark(LK_OBJ(FRAME->returnto));
    mark(LK_OBJ(FRAME->first));
    mark(LK_OBJ(FRAME->next));
    mark(LK_OBJ(FRAME->current));
    mark(LK_OBJ(FRAME->func));
}
static LK_OBJ_DEFFREEFUNC(free__fra) {
    if(LIST_ISINIT(FRAMESTACK)) darray_fin(FRAMESTACK);
}
LK_EXT_DEFINIT(lk_frame_extinittypes) {
    vm->t_frame = lk_object_allocwithsize(vm->t_obj, sizeof(lk_frame_t));
    lk_object_setmarkfunc(vm->t_frame, mark__fra);
    lk_object_setfreefunc(vm->t_frame, free__fra);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(Dargs__fra) {
    if(!LIST_ISINIT(FRAMESTACK)) RETURN(lk_list_new(VM));
    else {
        lk_list_t *args = lk_list_newfromlist(VM, FRAMESTACK);
        darray_limit(LIST(args), FRAME->argc);
        RETURN(args);
    }
}
LK_LIBRARY_DEFINECFUNCTION(DassignB__fra_str_obj) {
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
LK_LIBRARY_DEFINECFUNCTION(include__fra_str_str) {
    lk_frame_t *fr = lk_vm_evalfile(VM,
    darray_toCString(LIST(ARG(0))), darray_toCString(LIST(ARG(1))));
    if(fr != NULL) {
        qphash_t *from = fr->obj.slots;
        if(from != NULL) {
            qphash_t *to = self->obj.slots;
            if(to == NULL) to = self->obj.slots = qphash_alloc(
            sizeof(struct lk_slot), lk_object_hashcode, lk_object_keycmp);
            SET_EACH(from, i,
                *LK_SLOT(qphash_set(to, i->key)) = *LK_SLOT(SETITEM_VALUEPTR(i));
            );
        }
    }
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(raise__fra_err) {
    lk_vm_raiseerror(VM, LK_ERR(ARG(0)));
}
LK_LIBRARY_DEFINECFUNCTION(raise__fra_str) {
    lk_error_t *err = lk_error_new(VM, VM->t_error, NULL);
    err->text = LK_STRING(ARG(0));
    lk_vm_raiseerror(VM, err);
}
LK_LIBRARY_DEFINECFUNCTION(redo__fra) {
    if(LIST_ISINIT(FRAMESTACK)) darray_clear(FRAMESTACK);
    FRAME->next = FRAME->first;
    DONE;
}
LK_LIBRARY_DEFINECFUNCTION(require__fra_str_str) {
    RETURN(lk_vm_evalfile(VM,
    darray_toCString(LIST(ARG(0))), darray_toCString(LIST(ARG(1)))));
}
LK_LIBRARY_DEFINECFUNCTION(rescue__fra_f) {
    RETURN(lk_object_getvaluefromslot(self, lk_object_setslot(
    self, LK_OBJ(VM->str_rescue), VM->t_func, ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(RESOURCE__fra) {
    RETURN(VM->rsrc != NULL && !VM->rsrc->isstring
    ? LK_OBJ(VM->rsrc->rsrc) : N);
}
LK_LIBRARY_DEFINECFUNCTION(retry__fra) {
    lk_frame_t *caller = FRAME->caller;
    lk_instr_t *i = caller->current->prev;
    FRAME->next = NULL;
    FRAME->returnto = caller;
    while(i->prev != NULL && !(i->prev->opts & LK_INSTROEND)) i = i->prev;
    caller->next = i;
    DONE;
}
LK_LIBRARY_DEFINECFUNCTION(return__fra) {
    lk_frame_t *f = FRAME;
    for(; ; f = LK_OBJ_PROTO(f)) {
        if(f == NULL) lk_vm_abort(VM, NULL);
        if(CHKOPT(LK_FUNC(f->func)->cf.opts, LK_FUNCOASSIGNED)) break;
    }
    f = f->returnto;
    FRAME->next = NULL;
    FRAME->returnto = f;
    if(LIST_ISINIT(&env->stack)) {
        if(!LIST_ISINIT(FRAMESTACK)) darray_initptr(FRAMESTACK);
        darray_concat(FRAMESTACK, &env->stack);
    }
    DONE;
}
LK_EXT_DEFINIT(lk_frame_extinitfuncs) {
    lk_object_t *fra = vm->t_frame, *obj = vm->t_obj, *instr = vm->t_instr,
                *str = vm->t_string, *err = vm->t_error, *f = vm->t_func;
    lk_library_set(vm->t_vm, "Frame", fra);
    lk_library_setCFunction(fra, ".args", Dargs__fra, NULL);
    lk_library_setCFunction(fra, "=", DassignB__fra_str_obj, str, obj, NULL);
    lk_library_cfield(fra, ".caller", fra, offsetof(lk_frame_t, caller));
    lk_library_cfield(fra, ".current", instr, offsetof(lk_frame_t, current));
    lk_library_cfield(fra, ".first", instr, offsetof(lk_frame_t, first));
    lk_library_cfield(fra, ".frame", obj, offsetof(lk_frame_t, frame));
    lk_library_cfield(fra, ".function", obj, offsetof(lk_frame_t, func));
    lk_library_cfield(fra, ".next", instr, offsetof(lk_frame_t, next));
    lk_library_cfield(fra, ".receiver", obj, offsetof(lk_frame_t, receiver));
    lk_library_cfield(fra, ".return_to", fra, offsetof(lk_frame_t, returnto));
    lk_library_setCFunction(fra, "include", include__fra_str_str, str, str, NULL);
    lk_library_setCFunction(fra, "raise", raise__fra_err, err, NULL);
    lk_library_setCFunction(fra, "raise", raise__fra_str, str, NULL);
    lk_library_cfield(fra, "receiver", obj, offsetof(lk_frame_t, receiver));
    lk_library_setCFunction(fra, "redo", redo__fra, NULL);
    lk_library_setCFunction(fra, "require", require__fra_str_str, str, str, NULL);
    lk_library_setCFunction(fra, "rescue", rescue__fra_f, f, NULL);
    lk_library_setCFunction(fra, "RESOURCE", RESOURCE__fra, NULL);
    lk_library_setCFunction(fra, "retry", retry__fra, NULL);
    lk_library_setCFunction(fra, "return", return__fra, -1);
}

/* create a new frame based on the current one set in vm */
lk_frame_t *lk_frame_new(lk_vm_t *vm) {
    lk_frame_t *parent = vm->currentFrame;
    lk_frame_t *self;

    /* optimization to reduce the number of frames created */
    if(parent->child != NULL && parent->child->obj.mark.isref == 0) {
        vm->stat.recycledFrames ++;
        self = parent->child;
        self->obj.parent = LK_OBJ(parent);
        darray_clear(&self->stack);
        if(self->obj.slots != NULL) {
            qphash_clear(self->obj.slots);
        }
    } else {
        self = parent->child = LK_FRAME(lk_object_alloc(LK_OBJ(parent)));
    }

    /* init frame struct */
    vm->stat.totalFrames ++;
    vm->currentFrame = self;
    self->type = LK_FRAMETYPE_RETURN;
    self->frame = self;
    self->receiver = LK_OBJ(self);
    self->self = parent != NULL ? parent->self : NULL;
    self->caller = self->returnto = parent;
    return self;
}
void lk_frame_stackpush(lk_frame_t *self, lk_object_t *v) {
    assert(v != NULL);
    if(!LIST_ISINIT(&self->stack)) darray_initptr(&self->stack);
    darray_pushptr(&self->stack, lk_object_addref(LK_OBJ(self), v));
}

/* update */
lk_object_t *lk_frame_stackpop(lk_frame_t *self) {
    assert(LIST_ISINIT(&self->stack));
    assert(LIST_COUNT(&self->stack) > 0);
    return darray_popptr(&self->stack);
}
lk_object_t *lk_frame_stackpeek(lk_frame_t *self) {
    lk_vm_t *vm = LK_VM(self);
    if(!LIST_ISINIT(&self->stack)) return vm->t_nil;
    if(LIST_COUNT(&self->stack) < 1) return vm->t_nil;
    return darray_peekptr(&self->stack);
}
lk_list_t *lk_frame_stacktolist(lk_frame_t *self) {
    lk_list_t *stack = lk_list_new(LK_VM(self));
    if(LIST_ISINIT(&self->stack)) darray_copy(LIST(stack), &self->stack);
    return stack;
}
