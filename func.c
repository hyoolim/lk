#include "func.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
/* */
static LK_OBJ_DEFALLOCFUNC(alloc__f) {
    LK_FUNC(self)->cf.sigdef = LK_FUNC(parent)->cf.sigdef;
    LK_FUNC(self)->cf.minargc = LK_FUNC(parent)->cf.minargc;
    LK_FUNC(self)->cf.maxargc = LK_FUNC(parent)->cf.maxargc;
    if(LK_FUNC(self)->cf.sigs != NULL
    ) LK_FUNC(self)->cf.sigs = darray_clone(LK_FUNC(parent)->cf.sigs);
    LK_FUNC(self)->cf.rest = LK_FUNC(parent)->cf.rest;
    LK_FUNC(self)->cf.opts = LK_FUNC(parent)->cf.opts;
}
static LK_OBJ_DEFMARKFUNC(mark__f) {
    mark(LK_OBJ(LK_FUNC(self)->cf.sigdef));
    if(LK_FUNC(self)->cf.sigs != NULL
    ) LIST_EACHPTR(LK_FUNC(self)->cf.sigs, i, v, mark(v));
    mark(LK_OBJ(LK_FUNC(self)->cf.rest));
}
static LK_OBJ_DEFFREEFUNC(free__f) {
    if(LK_FUNC(self)->cf.sigs != NULL) darray_free(LK_FUNC(self)->cf.sigs);
}

/* */
static LK_OBJ_DEFALLOCFUNC(alloc__cf) {
    alloc__f(self, parent);
    LK_CFUNC(self)->func = LK_CFUNC(parent)->func;
}

/* */
static LK_OBJ_DEFALLOCFUNC(alloc__gf) {
    alloc__f(self, parent);
    LK_GFUNC(self)->funcs = darray_clone(LK_GFUNC(parent)->funcs);
}
static LK_OBJ_DEFMARKFUNC(mark__gf) {
    mark__f(self, mark);
    LIST_EACHPTR(LK_GFUNC(self)->funcs, i, v, mark(v));
}
static LK_OBJ_DEFFREEFUNC(free__gf) {
    free__f(self);
    darray_free(LK_GFUNC(self)->funcs);
}

/* */
static LK_OBJ_DEFALLOCFUNC(alloc__kf) {
    alloc__f(self, parent);
    LK_KFUNC(self)->frame = LK_KFUNC(parent)->frame;
    LK_KFUNC(self)->first = LK_KFUNC(parent)->first;
}
static LK_OBJ_DEFMARKFUNC(mark__kf) {
    mark__f(self, mark);
    mark(LK_OBJ(LK_KFUNC(self)->frame));
    mark(LK_OBJ(LK_KFUNC(self)->first));
}

/* */
static LK_OBJ_DEFALLOCFUNC(alloc__sig) {
    LK_SIG(self)->name = LK_SIG(parent)->name;
    LK_SIG(self)->check = LK_SIG(parent)->check;
    LK_SIG(self)->isself = LK_SIG(parent)->isself;
}
static LK_OBJ_DEFMARKFUNC(mark__sig) {
    mark(LK_OBJ(LK_SIG(self)->name));
    mark(LK_OBJ(LK_SIG(self)->check));
}
LK_EXT_DEFINIT(lk_func_extinittypes) {
    /* */
    vm->t_func = lk_object_allocwithsize(vm->t_obj, sizeof(lk_func_t));
    lk_object_setallocfunc(vm->t_func, alloc__f);
    lk_object_setmarkfunc(vm->t_func, mark__f);
    lk_object_setfreefunc(vm->t_func, free__f);
    /* */
    vm->t_cfunc = lk_object_allocwithsize(vm->t_func, sizeof(lk_cfunc_t));
    lk_object_setallocfunc(vm->t_cfunc, alloc__cf);
    /* */
    vm->t_gfunc = lk_object_allocwithsize(vm->t_func, sizeof(lk_gfunc_t));
    LK_GFUNC(vm->t_gfunc)->funcs = darray_alloc(sizeof(lk_func_t *), 4);
    lk_object_setallocfunc(vm->t_gfunc, alloc__gf);
    lk_object_setmarkfunc(vm->t_gfunc, mark__gf);
    lk_object_setfreefunc(vm->t_gfunc, free__gf);
    /* */
    vm->t_kfunc = lk_object_allocwithsize(vm->t_func, sizeof(lk_kfunc_t));
    lk_object_setallocfunc(vm->t_kfunc, alloc__kf);
    lk_object_setmarkfunc(vm->t_kfunc, mark__kf);
    /* */
    vm->t_sig = lk_object_allocwithsize(vm->t_obj, sizeof(lk_sig_t));
    lk_object_setallocfunc(vm->t_sig, alloc__sig);
    lk_object_setmarkfunc(vm->t_sig, mark__sig);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(add__f_f) {
    RETURN(lk_func_combine(LK_FUNC(self), LK_FUNC(ARG(0))));
}
LK_LIB_DEFINECFUNC(addB__f_f) {
    if(env->caller == NULL && env->caller->lastslot == NULL) {
        lk_vm_raisecstr(VM, "Cannot add to the function without a slot");
    } else {
        lk_gfunc_t *new = lk_func_combine(LK_FUNC(self), LK_FUNC(ARG(0)));
        lk_object_setvalueonslot(self, env->caller->lastslot, LK_OBJ(new));
        RETURN(new);
    }
}
LK_LIB_DEFINECFUNC(minimum_argument_size__f) {
    RETURN(lk_fi_new(VM, LK_FUNC(self)->cf.minargc));
}
LK_LIB_DEFINECFUNC(maximum_argument_size__f) {
    RETURN(lk_fi_new(VM, LK_FUNC(self)->cf.maxargc));
}
LK_LIB_DEFINECFUNC(signature__f) {
    darray_t *sigs = LK_FUNC(self)->cf.sigs;
    RETURN(sigs != NULL ? lk_list_newfromlist(VM, sigs) : lk_list_new(VM));
}
LK_LIB_DEFINECFUNC(last_argument__f) {
    lk_sig_t *last = LK_FUNC(self)->cf.rest;
    RETURN(last != NULL ? LK_OBJ(last) : NIL);
}
LK_LIB_DEFINECFUNC(functions__gf) {
    RETURN(lk_list_newfromlist(VM, LK_GFUNC(self)->funcs));
}
LK_LIB_DEFINECFUNC(name__sig) {
    lk_string_t *name = LK_SIG(self)->name;
    RETURN(name != NULL ? name : lk_string_newFromCString(VM, "_"));
}
LK_LIB_DEFINECFUNC(check__sig) {
    lk_object_t *type = LK_SIG(self)->check;
    RETURN(type != NULL ? type : VM->t_obj);
}
LK_EXT_DEFINIT(lk_func_extinitfuncs) {
    lk_object_t *f = vm->t_func, *sig = vm->t_sig;
    /* */
    lk_lib_setGlobal("Function", vm->t_func);
    lk_lib_setCFunc(f, "+", add__f_f, f, NULL);
    lk_lib_setCFunc(f, "+=", addB__f_f, f, NULL);
    lk_lib_setCField(f, "doc", f, offsetof(lk_func_t, cf.doc));
    lk_lib_setCFunc(f, "minimum_argument_size", minimum_argument_size__f, NULL);
    lk_lib_setCFunc(f, "maximum_argument_size", maximum_argument_size__f, NULL);
    lk_lib_setCFunc(f, "signature", signature__f, NULL);
    lk_lib_setCFunc(f, "last_argument", last_argument__f, NULL);
    /* */
    lk_lib_setGlobal("CFunction", vm->t_cfunc);
    /* */
    lk_lib_setGlobal("GenericFunction", vm->t_gfunc);
    lk_lib_setCFunc(vm->t_gfunc, "functions", functions__gf, NULL);
    /* */
    lk_lib_setGlobal("KineticFunction", vm->t_kfunc);
    /* */
    lk_lib_setGlobal("Signature", vm->t_sig);
    lk_lib_setCFunc(sig, "name", name__sig, NULL);
    lk_lib_setCFunc(sig, "check", check__sig, NULL);
}

/* new */
lk_cfunc_t *lk_cfunc_new(lk_vm_t *vm, lk_cfuncfunc_t *func, int minargc, int maxargc) {
    lk_cfunc_t *self = LK_CFUNC(lk_object_alloc(vm->t_cfunc));
    self->func = func;
    self->cf.minargc = minargc;
    self->cf.maxargc = maxargc;
    /* if(minargc > 0) self->cf.sigs = darray_allocptrwithcapacity(minargc); */
    self->cf.sigs = darray_allocptrwithcapacity(minargc);
    return self;
}
lk_kfunc_t *lk_kfunc_new(lk_vm_t *vm, lk_frame_t *frame, lk_instr_t *first) {
    lk_kfunc_t *self = LK_KFUNC(lk_object_alloc(vm->t_kfunc));
    self->frame = frame;
    self->first = first;
    self->cf.maxargc = INT_MAX;
    return self;
}
lk_gfunc_t *lk_gfunc_new(lk_vm_t *vm) {
    lk_gfunc_t *self = LK_GFUNC(lk_object_alloc(vm->t_gfunc));
    return self;
}
lk_sig_t *lk_sig_new(lk_vm_t *vm, lk_string_t *name, lk_object_t *type) {
    lk_sig_t *self = LK_SIG(lk_object_alloc(vm->t_sig));
    self->name = name;
    self->check = type;
    return self;
}

/* update */
lk_gfunc_t *lk_func_combine(lk_func_t *self, lk_func_t *other) {
    lk_vm_t *vm = LK_VM(self);
    darray_t *funcs;
    lk_kfunc_t *func;
    lk_sig_t *arg;
    int func_i, func_c, arg_i;
    int dist, other_dist = 0;
    if(!LK_OBJ_ISGFUNC(LK_OBJ(self))) {
        lk_gfunc_t *gf = lk_gfunc_new(vm);
        darray_pushptr(gf->funcs, self);
        if(CHKOPT(LK_FUNC(self)->cf.opts, LK_FUNCOASSIGNED)) {
            SETOPT(LK_FUNC(gf)->cf.opts, LK_FUNCOASSIGNED);
        }
        self = LK_FUNC(gf);
    }
    funcs = LK_GFUNC(self)->funcs;
    if(other->cf.sigs != NULL) {
        for(arg_i = 0; arg_i < LIST_COUNT(other->cf.sigs); arg_i ++) {
            arg = LIST_ATPTR(other->cf.sigs, arg_i);
            other_dist += lk_object_isa(arg->check, vm->t_obj);
        }
    }
    func_c = LIST_COUNT(funcs);
    for(func_i = 0; func_i < func_c; func_i ++) {
        func = LIST_ATPTR(funcs, func_i);
        dist = 0;
        if(func->cf.sigs != NULL) {
            for(arg_i = 0; arg_i < LIST_COUNT(func->cf.sigs); arg_i ++) {
                arg = LIST_ATPTR(func->cf.sigs, arg_i);
                dist += lk_object_isa(arg->check, vm->t_obj);
            }
        }
        if(other_dist >= dist) break;
    }
    if(func_i < func_c) darray_insertptr(funcs, func_i, other);
    else darray_pushptr(funcs, other);
    return LK_GFUNC(self);
}
lk_func_t *lk_func_match(lk_func_t *self, lk_frame_t *args, lk_object_t *recv) {
    lk_vm_t *vm = LK_VM(self);
    darray_t *funcs, *stack, *sigs;
    int funci, funcc, argi = 0, argc;
    lk_func_t *curr;
    lk_sig_t *sig;
    lk_object_t *arg;
    if(LK_OBJ_ISGFUNC(LK_OBJ(self))) {
        funcs = LK_GFUNC(self)->funcs;
        funci = 0; funcc = LIST_COUNT(funcs);
        curr = LIST_ATPTR(funcs, funci);
    } else {
        funcs = NULL;
        funci = 0; funcc = 1;
        curr = self;
    }
    findfunc:
    stack = &args->stack;
    argc = LIST_COUNT(stack);
    if(curr->cf.minargc <= argc && argc <= curr->cf.maxargc) {
        sigs = curr->cf.sigs;
        if(sigs != NULL) {
            for(; argi < LIST_COUNT(sigs); argi ++) {
                sig = LIST_ATPTR(sigs, argi);
                arg = LIST_ATPTR(stack, argi);
                if(sig == NULL || sig->check == NULL) goto bindarg;
                if(LK_OBJ_ISTYPE(arg, sig->check)) goto bindarg;
                goto nextfunc;
                bindarg:
                if(sig->name != NULL) lk_object_setslot(sig->isself
                ? recv : LK_OBJ(args), LK_OBJ(sig->name), sig->check, arg);
            }
        }
        if(curr->cf.rest != NULL) {
            sig = curr->cf.rest;
            if(sig != NULL && sig->name != NULL) {
                arg = LK_OBJ(LIST_ISINIT(stack)
                ? lk_list_newfromlist(vm, stack) : lk_list_new(vm));
                darray_offset(DARRAY(arg), argi);
                lk_object_setslot(sig->isself
                ? recv : LK_OBJ(args), LK_OBJ(sig->name), sig->check, arg);
            }
        }
        return curr;
    }
    nextfunc:
    if(++ funci >= funcc) return NULL;
    curr = LIST_ATPTR(funcs, funci);
    goto findfunc;
}
void lk_kfunc_updatesig(lk_kfunc_t *self) {
    lk_instr_t *typeinstr, *argdef = self->cf.sigdef;
    int isself;
    struct lk_slot *slot;
    darray_t *sigs = self->cf.sigs;
    lk_string_t *name;
    lk_object_t *type;
    lk_sig_t *sig;
    if(argdef != NULL && argdef->type == LK_INSTRTYPE_STRING) {
        self->cf.doc = LK_STRING(argdef->v);
        argdef = argdef->next;
    }
    for(; argdef != NULL; argdef = argdef->next) {
        if((isself = argdef->type == LK_INSTRTYPE_SELFMSG)
        || argdef->type == LK_INSTRTYPE_FRAMEMSG) {
            name = LK_STRING(argdef->v);
            if(darray_compareToCString(DARRAY(name), ":") == 0) {
                argdef = argdef->next;
                if(argdef != NULL && argdef->type == LK_INSTRTYPE_APPLY) {
                    typeinstr = LK_INSTR(argdef->v);
                    name = LK_STRING(typeinstr->v);
                    typeinstr = typeinstr->next;
                    slot = lk_object_getslotfromany(LK_OBJ(VM->currentFrame), typeinstr->v);
                    if(slot != NULL) {
                        type = lk_object_getvaluefromslot(LK_OBJ(VM->currentFrame), slot);
                    } else {
                        printf("Invalid sig, invalid type\n");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    printf("Invalid sig, expected type\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                type = VM->t_obj;
            }
            lk_object_addref(LK_OBJ(self), LK_OBJ(
            sig = lk_sig_new(VM, name, type)));
            sig->isself = isself;
            if(sigs == NULL) sigs = self->cf.sigs = darray_allocptr();
            if(LIST_COUNT(DARRAY(name)) > 3
            && darray_getuchar(DARRAY(name), -1) == '.'
            && darray_getuchar(DARRAY(name), -2) == '.'
            && darray_getuchar(DARRAY(name), -3) == '.') {
                sig->name = name = LK_STRING(lk_object_clone(LK_OBJ(name)));
                darray_limit(DARRAY(name), -3);
                self->cf.minargc = LIST_COUNT(sigs);
                self->cf.maxargc = INT_MAX;
                self->cf.rest = sig;
                return;
            } else {
                darray_pushptr(sigs, sig);
            }
        } else {
            printf("Invalid sig, expected name");
            exit(EXIT_FAILURE);
        }
    }
    self->cf.minargc =
    self->cf.maxargc = sigs != NULL ? LIST_COUNT(sigs) : 0;
}
