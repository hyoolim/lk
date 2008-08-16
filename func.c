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
    ) LK_FUNC(self)->cf.sigs = Sequence_clone(LK_FUNC(parent)->cf.sigs);
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
    if(LK_FUNC(self)->cf.sigs != NULL) Sequence_free(LK_FUNC(self)->cf.sigs);
}

/* */
static LK_OBJ_DEFALLOCFUNC(alloc__cf) {
    alloc__f(self, parent);
    LK_CFUNC(self)->func = LK_CFUNC(parent)->func;
}

/* */
static LK_OBJ_DEFALLOCFUNC(alloc__gf) {
    alloc__f(self, parent);
    LK_GFUNC(self)->funcs = Sequence_clone(LK_GFUNC(parent)->funcs);
}
static LK_OBJ_DEFMARKFUNC(mark__gf) {
    mark__f(self, mark);
    LIST_EACHPTR(LK_GFUNC(self)->funcs, i, v, mark(v));
}
static LK_OBJ_DEFFREEFUNC(free__gf) {
    free__f(self);
    Sequence_free(LK_GFUNC(self)->funcs);
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
LK_EXT_DEFINIT(lk_Func_extinittypes) {
    /* */
    vm->t_func = lk_Object_allocwithsize(vm->t_obj, sizeof(lk_Func_t));
    lk_Object_setallocfunc(vm->t_func, alloc__f);
    lk_Object_setmarkfunc(vm->t_func, mark__f);
    lk_Object_setfreefunc(vm->t_func, free__f);
    /* */
    vm->t_cfunc = lk_Object_allocwithsize(vm->t_func, sizeof(lk_Cfunc_t));
    lk_Object_setallocfunc(vm->t_cfunc, alloc__cf);
    /* */
    vm->t_gfunc = lk_Object_allocwithsize(vm->t_func, sizeof(lk_Gfunc_t));
    LK_GFUNC(vm->t_gfunc)->funcs = Sequence_alloc(sizeof(lk_Func_t *), 4);
    lk_Object_setallocfunc(vm->t_gfunc, alloc__gf);
    lk_Object_setmarkfunc(vm->t_gfunc, mark__gf);
    lk_Object_setfreefunc(vm->t_gfunc, free__gf);
    /* */
    vm->t_kfunc = lk_Object_allocwithsize(vm->t_func, sizeof(lk_Kfunc_t));
    lk_Object_setallocfunc(vm->t_kfunc, alloc__kf);
    lk_Object_setmarkfunc(vm->t_kfunc, mark__kf);
    /* */
    vm->t_sig = lk_Object_allocwithsize(vm->t_obj, sizeof(lk_Sig_t));
    lk_Object_setallocfunc(vm->t_sig, alloc__sig);
    lk_Object_setmarkfunc(vm->t_sig, mark__sig);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(add__f_f) {
    RETURN(lk_Func_combine(LK_FUNC(self), LK_FUNC(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(addB__f_f) {
    if(env->caller == NULL && env->caller->lastslot == NULL) {
        lk_Vm_raisecstr(VM, "Cannot add to the function without a slot");
    } else {
        lk_Gfunc_t *new = lk_Func_combine(LK_FUNC(self), LK_FUNC(ARG(0)));
        lk_Object_setvalueonslot(self, env->caller->lastslot, LK_OBJ(new));
        RETURN(new);
    }
}
LK_LIBRARY_DEFINECFUNCTION(minimum_argument_count__f) {
    RETURN(lk_Fi_new(VM, LK_FUNC(self)->cf.minargc));
}
LK_LIBRARY_DEFINECFUNCTION(maximum_argument_count__f) {
    RETURN(lk_Fi_new(VM, LK_FUNC(self)->cf.maxargc));
}
LK_LIBRARY_DEFINECFUNCTION(signature__f) {
    Sequence_t *sigs = LK_FUNC(self)->cf.sigs;
    RETURN(sigs != NULL ? lk_List_newfromlist(VM, sigs) : lk_List_new(VM));
}
LK_LIBRARY_DEFINECFUNCTION(last_argument__f) {
    lk_Sig_t *last = LK_FUNC(self)->cf.rest;
    RETURN(last != NULL ? LK_OBJ(last) : N);
}
LK_LIBRARY_DEFINECFUNCTION(functions__gf) {
    RETURN(lk_List_newfromlist(VM, LK_GFUNC(self)->funcs));
}
LK_LIBRARY_DEFINECFUNCTION(name__sig) {
    lk_String_t *name = LK_SIG(self)->name;
    RETURN(name != NULL ? name : lk_String_newfromcstr(VM, "_"));
}
LK_LIBRARY_DEFINECFUNCTION(check__sig) {
    lk_Object_t *type = LK_SIG(self)->check;
    RETURN(type != NULL ? type : VM->t_obj);
}
LK_EXT_DEFINIT(lk_Func_extinitfuncs) {
    lk_Object_t *f = vm->t_func, *sig = vm->t_sig;
    /* */
    lk_Library_setGlobal("Function", vm->t_func);
    lk_Library_setCFunction(f, "+", add__f_f, f, NULL);
    lk_Library_setCFunction(f, "+=", addB__f_f, f, NULL);
    lk_Library_cfield(f, "doc", f, offsetof(lk_Func_t, cf.doc));
    lk_Library_setCFunction(f, "minimum_argument_count", minimum_argument_count__f, NULL);
    lk_Library_setCFunction(f, "maximum_argument_count", maximum_argument_count__f, NULL);
    lk_Library_setCFunction(f, "signature", signature__f, NULL);
    lk_Library_setCFunction(f, "last_argument", last_argument__f, NULL);
    /* */
    lk_Library_setGlobal("CFunction", vm->t_cfunc);
    /* */
    lk_Library_setGlobal("GenericFunction", vm->t_gfunc);
    lk_Library_setCFunction(vm->t_gfunc, "functions", functions__gf, NULL);
    /* */
    lk_Library_setGlobal("KineticFunction", vm->t_kfunc);
    /* */
    lk_Library_setGlobal("Signature", vm->t_sig);
    lk_Library_setCFunction(sig, "name", name__sig, NULL);
    lk_Library_setCFunction(sig, "check", check__sig, NULL);
}

/* new */
lk_Cfunc_t *lk_Cfunc_new(lk_Vm_t *vm, lk_Cfuncfunc_t *func, int minargc, int maxargc) {
    lk_Cfunc_t *self = LK_CFUNC(lk_Object_alloc(vm->t_cfunc));
    self->func = func;
    self->cf.minargc = minargc;
    self->cf.maxargc = maxargc;
    /* if(minargc > 0) self->cf.sigs = Sequence_allocptrwithcapa(minargc); */
    self->cf.sigs = Sequence_allocptrwithcapa(minargc);
    return self;
}
lk_Kfunc_t *lk_Kfunc_new(lk_Vm_t *vm, lk_Frame_t *frame, lk_Instr_t *first) {
    lk_Kfunc_t *self = LK_KFUNC(lk_Object_alloc(vm->t_kfunc));
    self->frame = frame;
    self->first = first;
    self->cf.maxargc = INT_MAX;
    return self;
}
lk_Gfunc_t *lk_Gfunc_new(lk_Vm_t *vm) {
    lk_Gfunc_t *self = LK_GFUNC(lk_Object_alloc(vm->t_gfunc));
    return self;
}
lk_Sig_t *lk_Sig_new(lk_Vm_t *vm, lk_String_t *name, lk_Object_t *type) {
    lk_Sig_t *self = LK_SIG(lk_Object_alloc(vm->t_sig));
    self->name = name;
    self->check = type;
    return self;
}

/* update */
lk_Gfunc_t *lk_Func_combine(lk_Func_t *self, lk_Func_t *other) {
    lk_Vm_t *vm = LK_VM(self);
    Sequence_t *funcs;
    lk_Kfunc_t *func;
    lk_Sig_t *arg;
    int func_i, func_c, arg_i;
    int dist, other_dist = 0;
    if(!LK_OBJ_ISGFUNC(LK_OBJ(self))) {
        lk_Gfunc_t *gf = lk_Gfunc_new(vm);
        Sequence_pushptr(gf->funcs, self);
        if(CHKOPT(LK_FUNC(self)->cf.opts, LK_FUNCOASSIGNED)) {
            SETOPT(LK_FUNC(gf)->cf.opts, LK_FUNCOASSIGNED);
        }
        self = LK_FUNC(gf);
    }
    funcs = LK_GFUNC(self)->funcs;
    if(other->cf.sigs != NULL) {
        for(arg_i = 0; arg_i < LIST_COUNT(other->cf.sigs); arg_i ++) {
            arg = LIST_ATPTR(other->cf.sigs, arg_i);
            other_dist += lk_Object_isa(arg->check, vm->t_obj);
        }
    }
    func_c = LIST_COUNT(funcs);
    for(func_i = 0; func_i < func_c; func_i ++) {
        func = LIST_ATPTR(funcs, func_i);
        dist = 0;
        if(func->cf.sigs != NULL) {
            for(arg_i = 0; arg_i < LIST_COUNT(func->cf.sigs); arg_i ++) {
                arg = LIST_ATPTR(func->cf.sigs, arg_i);
                dist += lk_Object_isa(arg->check, vm->t_obj);
            }
        }
        if(other_dist >= dist) break;
    }
    if(func_i < func_c) Sequence_insertptr(funcs, func_i, other);
    else Sequence_pushptr(funcs, other);
    return LK_GFUNC(self);
}
lk_Func_t *lk_Func_match(lk_Func_t *self, lk_Frame_t *args, lk_Object_t *recv) {
    lk_Vm_t *vm = LK_VM(self);
    Sequence_t *funcs, *stack, *sigs;
    int funci, funcc, argi = 0, argc;
    lk_Func_t *curr;
    lk_Sig_t *sig;
    lk_Object_t *arg;
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
                if(sig->name != NULL) lk_Object_setslot(sig->isself
                ? recv : LK_OBJ(args), LK_OBJ(sig->name), sig->check, arg);
            }
        }
        if(curr->cf.rest != NULL) {
            sig = curr->cf.rest;
            if(sig != NULL && sig->name != NULL) {
                arg = LK_OBJ(LIST_ISINIT(stack)
                ? lk_List_newfromlist(vm, stack) : lk_List_new(vm));
                Sequence_offset(LIST(arg), argi);
                lk_Object_setslot(sig->isself
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
void lk_Kfunc_updatesig(lk_Kfunc_t *self) {
    lk_Instr_t *typeinstr, *argdef = self->cf.sigdef;
    int isself;
    struct lk_Slot *slot;
    Sequence_t *sigs = self->cf.sigs;
    lk_String_t *name;
    lk_Object_t *type;
    lk_Sig_t *sig;
    if(argdef != NULL && argdef->type == LK_INSTRTYPE_STRING) {
        self->cf.doc = LK_STRING(argdef->v);
        argdef = argdef->next;
    }
    for(; argdef != NULL; argdef = argdef->next) {
        if((isself = argdef->type == LK_INSTRTYPE_SELFMSG)
        || argdef->type == LK_INSTRTYPE_FRAMEMSG) {
            name = LK_STRING(argdef->v);
            if(Sequence_cmpcstr(LIST(name), ":") == 0) {
                argdef = argdef->next;
                if(argdef != NULL && argdef->type == LK_INSTRTYPE_APPLY) {
                    typeinstr = LK_INSTR(argdef->v);
                    name = LK_STRING(typeinstr->v);
                    typeinstr = typeinstr->next;
                    slot = lk_Object_getslotfromany(LK_OBJ(VM->currentFrame), typeinstr->v);
                    if(slot != NULL) {
                        type = lk_Object_getvaluefromslot(LK_OBJ(VM->currentFrame), slot);
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
            lk_Object_addref(LK_OBJ(self), LK_OBJ(
            sig = lk_Sig_new(VM, name, type)));
            sig->isself = isself;
            if(sigs == NULL) sigs = self->cf.sigs = Sequence_allocptr();
            if(LIST_COUNT(LIST(name)) > 3
            && Sequence_getuchar(LIST(name), -1) == '.'
            && Sequence_getuchar(LIST(name), -2) == '.'
            && Sequence_getuchar(LIST(name), -3) == '.') {
                sig->name = name = LK_STRING(lk_Object_clone(LK_OBJ(name)));
                Sequence_limit(LIST(name), -3);
                self->cf.minargc = LIST_COUNT(sigs);
                self->cf.maxargc = INT_MAX;
                self->cf.rest = sig;
                return;
            } else {
                Sequence_pushptr(sigs, sig);
            }
        } else {
            printf("Invalid sig, expected name");
            exit(EXIT_FAILURE);
        }
    }
    self->cf.minargc =
    self->cf.maxargc = sigs != NULL ? LIST_COUNT(sigs) : 0;
}
