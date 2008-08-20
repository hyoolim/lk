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
LK_LIB_DEFINEINIT(lk_frame_libPreInit) {
    vm->t_frame = lk_object_allocwithsize(vm->t_obj, sizeof(lk_frame_t));
    lk_object_setmarkfunc(vm->t_frame, mark__fra);
    lk_object_setfreefunc(vm->t_frame, free__fra);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(Dargs__fra) {
    if(!LIST_ISINIT(FRAMESTACK)) RETURN(lk_list_new(VM));
    else {
        lk_list_t *args = lk_list_newfromlist(VM, FRAMESTACK);
        darray_limit(DARRAY(args), FRAME->argc);
        RETURN(args);
    }
}
LK_LIB_DEFINECFUNC(DassignB__fra_str_obj) {
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
LK_LIB_DEFINECFUNC(include__fra_str_str) {
    lk_frame_t *fr = lk_vm_evalfile(VM,
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
LK_LIB_DEFINECFUNC(raise__fra_err) {
    lk_vm_raiseerror(VM, LK_ERR(ARG(0)));
}
LK_LIB_DEFINECFUNC(raise__fra_str) {
    lk_error_t *err = lk_error_new(VM, VM->t_error, NULL);
    err->text = LK_STRING(ARG(0));
    lk_vm_raiseerror(VM, err);
}
LK_LIB_DEFINECFUNC(redo__fra) {
    if(LIST_ISINIT(FRAMESTACK)) darray_clear(FRAMESTACK);
    FRAME->next = FRAME->first;
    DONE;
}
LK_LIB_DEFINECFUNC(require__fra_str_str) {
    RETURN(lk_vm_evalfile(VM,
    darray_toCString(DARRAY(ARG(0))), darray_toCString(DARRAY(ARG(1)))));
}
LK_LIB_DEFINECFUNC(rescue__fra_f) {
    RETURN(lk_object_getvaluefromslot(self, lk_object_setslot(
    self, LK_OBJ(VM->str_rescue), VM->t_func, ARG(0))));
}
LK_LIB_DEFINECFUNC(RESOURCE__fra) {
    RETURN(VM->rsrc != NULL && !VM->rsrc->isstring
    ? LK_OBJ(VM->rsrc->rsrc) : NIL);
}
LK_LIB_DEFINECFUNC(retry__fra) {
    lk_frame_t *caller = FRAME->caller;
    lk_instr_t *i = caller->current->prev;
    FRAME->next = NULL;
    FRAME->returnto = caller;
    while(i->prev != NULL && !(i->prev->opts & LK_INSTROEND)) i = i->prev;
    caller->next = i;
    DONE;
}
LK_LIB_DEFINECFUNC(return__fra) {
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
LK_LIB_DEFINEINIT(lk_frame_libInit) {
    lk_object_t *fra = vm->t_frame, *obj = vm->t_obj, *instr = vm->t_instr,
                *str = vm->t_string, *err = vm->t_error, *f = vm->t_func;
    lk_lib_setObject(vm->t_vm, "Frame", fra);
    lk_lib_setCFunc(fra, ".args", Dargs__fra, NULL);
    lk_lib_setCFunc(fra, "=", DassignB__fra_str_obj, str, obj, NULL);
    lk_lib_setCField(fra, ".caller", fra, offsetof(lk_frame_t, caller));
    lk_lib_setCField(fra, ".current", instr, offsetof(lk_frame_t, current));
    lk_lib_setCField(fra, ".first", instr, offsetof(lk_frame_t, first));
    lk_lib_setCField(fra, ".frame", obj, offsetof(lk_frame_t, frame));
    lk_lib_setCField(fra, ".function", obj, offsetof(lk_frame_t, func));
    lk_lib_setCField(fra, ".next", instr, offsetof(lk_frame_t, next));
    lk_lib_setCField(fra, ".receiver", obj, offsetof(lk_frame_t, receiver));
    lk_lib_setCField(fra, ".return_to", fra, offsetof(lk_frame_t, returnto));
    lk_lib_setCFunc(fra, "include", include__fra_str_str, str, str, NULL);
    lk_lib_setCFunc(fra, "raise", raise__fra_err, err, NULL);
    lk_lib_setCFunc(fra, "raise", raise__fra_str, str, NULL);
    lk_lib_setCField(fra, "receiver", obj, offsetof(lk_frame_t, receiver));
    lk_lib_setCFunc(fra, "redo", redo__fra, NULL);
    lk_lib_setCFunc(fra, "require", require__fra_str_str, str, str, NULL);
    lk_lib_setCFunc(fra, "rescue", rescue__fra_f, f, NULL);
    lk_lib_setCFunc(fra, "RESOURCE", RESOURCE__fra, NULL);
    lk_lib_setCFunc(fra, "retry", retry__fra, NULL);
    lk_lib_setCFunc(fra, "return", return__fra, -1);
}

/* create a new frame based on the current one set in vm */
lk_frame_t *lk_frame_new(lk_vm_t *vm) {
    lk_frame_t *parent = vm->currentFrame;
    lk_frame_t *self;

    /* optimization to reduce the number of frames created */
    if(parent->child != NULL && parent->child->o.mark.isref == 0) {
        vm->stat.recycledFrames ++;
        self = parent->child;
        self->o.parent = LK_OBJ(parent);
        darray_clear(&self->stack);
        if(self->o.slots != NULL) {
            qphash_clear(self->o.slots);
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
    if(LIST_ISINIT(&self->stack)) darray_copy(DARRAY(stack), &self->stack);
    return stack;
}
