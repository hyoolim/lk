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
static LK_OBJECT_DEFMARKFUNC(mark__fra) {
    if(LIST_ISINIT(FRAMESTACK)) LIST_EACHPTR(FRAMESTACK, i, v, mark(v));
    mark(LK_O(FRAME->frame));
    mark(LK_O(FRAME->receiver));
    mark(LK_O(FRAME->self));
    mark(LK_O(FRAME->caller));
    mark(LK_O(FRAME->child));
    mark(LK_O(FRAME->returnto));
    mark(LK_O(FRAME->first));
    mark(LK_O(FRAME->next));
    mark(LK_O(FRAME->current));
    mark(LK_O(FRAME->func));
}
static LK_OBJECT_DEFFREEFUNC(free__fra) {
    if(LIST_ISINIT(FRAMESTACK)) list_fin(FRAMESTACK);
}
LK_EXT_DEFINIT(lk_frame_extinittypes) {
    vm->t_frame = lk_object_allocwithsize(vm->t_object, sizeof(lk_frame_t));
    lk_object_setmarkfunc(vm->t_frame, mark__fra);
    lk_object_setfreefunc(vm->t_frame, free__fra);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(Dargs__fra) {
    if(!LIST_ISINIT(FRAMESTACK)) RETURN(lk_list_new(VM));
    else {
        lk_list_t *args = lk_list_newfromlist(VM, FRAMESTACK);
        list_limit(LIST(args), FRAME->argc);
        RETURN(args);
    }
}
static LK_EXT_DEFCFUNC(DassignB__fra_str_obj) {
    lk_string_t *k = LK_STRING(ARG(0));
    lk_object_t *v = ARG(1);
    struct lk_slotv *sv = lk_object_getdef(self, LK_O(k));
    lk_object_t *oldval;
    if(sv == NULL) lk_vm_raisecstr(VM, "Cannot assign to %s without defining it first", k);
    oldval = lk_object_getslotv(self, sv);
    if(LK_OBJECT_ISFUNC(oldval)
    ) v = LK_O(lk_func_combine(LK_FUNC(oldval), LK_FUNC(v)));
    lk_object_setslotv(self, sv, LK_O(k), v);
    v = lk_object_getslotv(self, sv);
    if(LK_OBJECT_ISFUNC(v)) {
        SETOPT(sv->opts, LK_SLOTVOAUTORUN);
        SETOPT(LK_FUNC(v)->cf.opts, LK_FUNCOASSIGNED);
    }
    RETURN(v);
}
static LK_EXT_DEFCFUNC(include__fra_str_str) {
    lk_frame_t *fr = lk_vm_evalfile(VM,
    list_tocstr(LIST(ARG(0))), list_tocstr(LIST(ARG(1))));
    if(fr != NULL) {
        set_t *from = fr->co.slots;
        if(from != NULL) {
            set_t *to = self->co.slots;
            if(to == NULL) to = self->co.slots = set_alloc(
            sizeof(struct lk_slotv), lk_object_hashcode, lk_object_keycmp);
            SET_EACH(from, i,
                *LK_SLOTV(set_set(to, i->key)) = *LK_SLOTV(SETITEM_VALUEPTR(i));
            );
        }
    }
    RETURN(self);
}
static LK_EXT_DEFCFUNC(raise__fra_err) {
    lk_vm_raiseerror(VM, LK_ERROR(ARG(0)));
}
static LK_EXT_DEFCFUNC(raise__fra_str) {
    lk_error_t *err = lk_error_new(VM, VM->t_error, NULL);
    err->text = LK_STRING(ARG(0));
    lk_vm_raiseerror(VM, err);
}
static LK_EXT_DEFCFUNC(redo__fra) {
    if(LIST_ISINIT(FRAMESTACK)) list_clear(FRAMESTACK);
    FRAME->next = FRAME->first;
    DONE;
}
static LK_EXT_DEFCFUNC(require__fra_str_str) {
    RETURN(lk_vm_evalfile(VM,
    list_tocstr(LIST(ARG(0))), list_tocstr(LIST(ARG(1)))));
}
static LK_EXT_DEFCFUNC(rescue__fra_f) {
    RETURN(lk_object_getslotv(self, lk_object_setslot(
    self, LK_O(VM->str_rescue), VM->t_func, ARG(0))));
}
static LK_EXT_DEFCFUNC(RESOURCE__fra) {
    RETURN(VM->rsrc != NULL && !VM->rsrc->isstring
    ? LK_O(VM->rsrc->rsrc) : N);
}
static LK_EXT_DEFCFUNC(retry__fra) {
    lk_frame_t *caller = FRAME->caller;
    lk_instr_t *i = caller->current->prev;
    FRAME->next = NULL;
    FRAME->returnto = caller;
    while(i->prev != NULL && !(i->prev->opts & LK_INSTROEND)) i = i->prev;
    caller->next = i;
    DONE;
}
static LK_EXT_DEFCFUNC(return__fra) {
    lk_frame_t *f = FRAME;
    for(; ; f = LK_OBJECT_PROTO(f)) {
        if(f == NULL) lk_vm_abort(VM, NULL);
        if(CHKOPT(LK_FUNC(f->func)->cf.opts, LK_FUNCOASSIGNED)) break;
    }
    f = f->returnto;
    FRAME->next = NULL;
    FRAME->returnto = f;
    if(LIST_ISINIT(&env->stack)) {
        if(!LIST_ISINIT(FRAMESTACK)) list_initptr(FRAMESTACK);
        list_concat(FRAMESTACK, &env->stack);
    }
    DONE;
}
LK_EXT_DEFINIT(lk_frame_extinitfuncs) {
    lk_object_t *fra = vm->t_frame, *obj = vm->t_object, *instr = vm->t_instr,
                *str = vm->t_string, *err = vm->t_error, *f = vm->t_func;
    lk_ext_set(vm->t_vm, "Frame", fra);
    lk_ext_cfunc(fra, ".args", Dargs__fra, NULL);
    lk_ext_cfunc(fra, ".assign!", DassignB__fra_str_obj, str, obj, NULL);
    lk_ext_cfield(fra, ".caller", fra, offsetof(lk_frame_t, caller));
    lk_ext_cfield(fra, ".current", instr, offsetof(lk_frame_t, current));
    lk_ext_cfield(fra, ".first", instr, offsetof(lk_frame_t, first));
    lk_ext_cfield(fra, ".frame", obj, offsetof(lk_frame_t, frame));
    lk_ext_cfield(fra, ".function", obj, offsetof(lk_frame_t, func));
    lk_ext_cfield(fra, ".next", instr, offsetof(lk_frame_t, next));
    lk_ext_cfield(fra, ".receiver", obj, offsetof(lk_frame_t, receiver));
    lk_ext_cfield(fra, ".return_to", fra, offsetof(lk_frame_t, returnto));
    lk_ext_cfunc(fra, "include", include__fra_str_str, str, str, NULL);
    lk_ext_cfunc(fra, "raise", raise__fra_err, err, NULL);
    lk_ext_cfunc(fra, "raise", raise__fra_str, str, NULL);
    lk_ext_cfield(fra, "receiver", obj, offsetof(lk_frame_t, receiver));
    lk_ext_cfunc(fra, "redo", redo__fra, NULL);
    lk_ext_cfunc(fra, "require", require__fra_str_str, str, str, NULL);
    lk_ext_cfunc(fra, "rescue", rescue__fra_f, f, NULL);
    lk_ext_cfunc(fra, "RESOURCE", RESOURCE__fra, NULL);
    lk_ext_cfunc(fra, "retry", retry__fra, NULL);
    lk_ext_cfunc(fra, "return", return__fra, -1);
}

/* update */
void lk_frame_stackpush(lk_frame_t *self, lk_object_t *v) {
    assert(v != NULL);
    if(!LIST_ISINIT(&self->stack)) list_initptr(&self->stack);
    list_pushptr(&self->stack, lk_object_addref(LK_O(self), v));
}
lk_object_t *lk_frame_stackpop(lk_frame_t *self) {
    assert(LIST_ISINIT(&self->stack));
    assert(LIST_COUNT(&self->stack) > 0);
    return list_popptr(&self->stack);
}
lk_object_t *lk_frame_stackpeek(lk_frame_t *self) {
    lk_vm_t *vm = LK_VM(self);
    if(!LIST_ISINIT(&self->stack)) return vm->t_unknown;
    if(LIST_COUNT(&self->stack) < 1) return vm->t_unknown;
    return list_peekptr(&self->stack);
}
lk_list_t *lk_frame_stacktolist(lk_frame_t *self) {
    lk_list_t *stack = lk_list_new(LK_VM(self));
    if(LIST_ISINIT(&self->stack)) list_copy(LIST(stack), &self->stack);
    return stack;
}
