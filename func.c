#include "func.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
/* */
static LK_OBJECT_DEFALLOCFUNC(alloc__f) {
    LK_FUNC(self)->cf.sigdef = LK_FUNC(proto)->cf.sigdef;
    LK_FUNC(self)->cf.minargc = LK_FUNC(proto)->cf.minargc;
    LK_FUNC(self)->cf.maxargc = LK_FUNC(proto)->cf.maxargc;
    if(LK_FUNC(self)->cf.sigs != NULL
    ) LK_FUNC(self)->cf.sigs = list_clone(LK_FUNC(proto)->cf.sigs);
    LK_FUNC(self)->cf.rest = LK_FUNC(proto)->cf.rest;
    LK_FUNC(self)->cf.opts = LK_FUNC(proto)->cf.opts;
}
static LK_OBJECT_DEFMARKFUNC(mark__f) {
    mark(LK_O(LK_FUNC(self)->cf.sigdef));
    if(LK_FUNC(self)->cf.sigs != NULL
    ) LIST_EACHPTR(LK_FUNC(self)->cf.sigs, i, v, mark(v));
    mark(LK_O(LK_FUNC(self)->cf.rest));
}
static LK_OBJECT_DEFFREEFUNC(free__f) {
    if(LK_FUNC(self)->cf.sigs != NULL) list_free(LK_FUNC(self)->cf.sigs);
}

/* */
static LK_OBJECT_DEFALLOCFUNC(alloc__cf) {
    alloc__f(self, proto);
    LK_CFUNC(self)->func = LK_CFUNC(proto)->func;
}

/* */
static LK_OBJECT_DEFALLOCFUNC(alloc__gf) {
    alloc__f(self, proto);
    LK_GFUNC(self)->funcs = list_clone(LK_GFUNC(proto)->funcs);
}
static LK_OBJECT_DEFMARKFUNC(mark__gf) {
    mark__f(self, mark);
    LIST_EACHPTR(LK_GFUNC(self)->funcs, i, v, mark(v));
}
static LK_OBJECT_DEFFREEFUNC(free__gf) {
    free__f(self);
    list_free(LK_GFUNC(self)->funcs);
}

/* */
static LK_OBJECT_DEFALLOCFUNC(alloc__kf) {
    alloc__f(self, proto);
    LK_KFUNC(self)->frame = LK_KFUNC(proto)->frame;
    LK_KFUNC(self)->first = LK_KFUNC(proto)->first;
}
static LK_OBJECT_DEFMARKFUNC(mark__kf) {
    mark__f(self, mark);
    mark(LK_O(LK_KFUNC(self)->frame));
    mark(LK_O(LK_KFUNC(self)->first));
}

/* */
static LK_OBJECT_DEFALLOCFUNC(alloc__sig) {
    LK_SIG(self)->name = LK_SIG(proto)->name;
    LK_SIG(self)->type = LK_SIG(proto)->type;
    LK_SIG(self)->isself = LK_SIG(proto)->isself;
}
static LK_OBJECT_DEFMARKFUNC(mark__sig) {
    mark(LK_O(LK_SIG(self)->name));
    mark(LK_O(LK_SIG(self)->type));
}
LK_EXT_DEFINIT(lk_func_extinittypes) {
    /* */
    vm->t_func = lk_object_allocwithsize(vm->t_object, sizeof(lk_func_t));
    lk_object_setallocfunc(vm->t_func, alloc__f);
    lk_object_setmarkfunc(vm->t_func, mark__f);
    lk_object_setfreefunc(vm->t_func, free__f);
    /* */
    vm->t_cfunc = lk_object_allocwithsize(vm->t_func, sizeof(lk_cfunc_t));
    lk_object_setallocfunc(vm->t_cfunc, alloc__cf);
    /* */
    vm->t_gfunc = lk_object_allocwithsize(vm->t_func, sizeof(lk_gfunc_t));
    LK_GFUNC(vm->t_gfunc)->funcs = list_alloc(sizeof(lk_func_t *), 4);
    lk_object_setallocfunc(vm->t_gfunc, alloc__gf);
    lk_object_setmarkfunc(vm->t_gfunc, mark__gf);
    lk_object_setfreefunc(vm->t_gfunc, free__gf);
    /* */
    vm->t_kfunc = lk_object_allocwithsize(vm->t_func, sizeof(lk_kfunc_t));
    lk_object_setallocfunc(vm->t_kfunc, alloc__kf);
    lk_object_setmarkfunc(vm->t_kfunc, mark__kf);
    /* */
    vm->t_sig = lk_object_allocwithsize(vm->t_object, sizeof(lk_sig_t));
    lk_object_setallocfunc(vm->t_sig, alloc__sig);
    lk_object_setmarkfunc(vm->t_sig, mark__sig);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(add__f_f) {
    RETURN(lk_func_combine(LK_FUNC(self), LK_FUNC(ARG(0))));
}
static LK_EXT_DEFCFUNC(addB__f_f) {
    if(env->caller == NULL && env->caller->lastslot == NULL) {
        lk_vm_raisecstr(VM, "Cannot add to the function without a slot");
    } else {
        lk_func_t *new = lk_func_combine(LK_FUNC(self), LK_FUNC(ARG(0)));
        lk_object_setslotvalue(self, env->caller->lastslot, NULL, new);
        RETURN(new);
    }
}
static LK_EXT_DEFCFUNC(minimum_argument_count__f) {
    RETURN(lk_fi_new(VM, LK_FUNC(self)->cf.minargc));
}
static LK_EXT_DEFCFUNC(maximum_argument_count__f) {
    RETURN(lk_fi_new(VM, LK_FUNC(self)->cf.maxargc));
}
static LK_EXT_DEFCFUNC(signature__f) {
    list_t *sigs = LK_FUNC(self)->cf.sigs;
    RETURN(sigs != NULL ? lk_list_newfromlist(VM, sigs) : lk_list_new(VM));
}
static LK_EXT_DEFCFUNC(last_argument__f) {
    lk_sig_t *last = LK_FUNC(self)->cf.rest;
    RETURN(last != NULL ? LK_O(last) : N);
}
static LK_EXT_DEFCFUNC(functions__gf) {
    RETURN(lk_list_newfromlist(VM, LK_GFUNC(self)->funcs));
}
static LK_EXT_DEFCFUNC(name__sig) {
    lk_string_t *name = LK_SIG(self)->name;
    RETURN(name != NULL ? name : lk_string_newfromcstr(VM, "_"));
}
static LK_EXT_DEFCFUNC(type__sig) {
    lk_object_t *type = LK_SIG(self)->type;
    RETURN(type != NULL ? type : VM->t_object);
}
LK_EXT_DEFINIT(lk_func_extinitfuncs) {
    lk_object_t *f = vm->t_func, *sig = vm->t_sig;
    /* */
    lk_ext_global("Function", vm->t_func);
    lk_ext_cfunc(f, "add", add__f_f, f, NULL);
    lk_ext_cfunc(f, "add=", addB__f_f, f, NULL);
    lk_ext_cfield(f, "doc", f, offsetof(lk_func_t, cf.doc));
    lk_ext_cfunc(f, "minimum_argument_count", minimum_argument_count__f, NULL);
    lk_ext_cfunc(f, "maximum_argument_count", maximum_argument_count__f, NULL);
    lk_ext_cfunc(f, "signature", signature__f, NULL);
    lk_ext_cfunc(f, "last_argument", last_argument__f, NULL);
    /* */
    lk_ext_global("CFunction", vm->t_cfunc);
    /* */
    lk_ext_global("GenericFunction", vm->t_gfunc);
    lk_ext_cfunc(vm->t_gfunc, "functions", functions__gf, NULL);
    /* */
    lk_ext_global("KineticFunction", vm->t_kfunc);
    /* */
    lk_ext_global("Signature", vm->t_sig);
    lk_ext_cfunc(sig, "name", name__sig, NULL);
    lk_ext_cfunc(sig, "type", type__sig, NULL);
}

/* new */
lk_cfunc_t *lk_cfunc_new(lk_vm_t *vm, lk_cfuncfunc_t *func, int minargc, int maxargc) {
    lk_cfunc_t *self = LK_CFUNC(lk_object_alloc(vm->t_cfunc));
    self->func = func;
    self->cf.minargc = minargc;
    self->cf.maxargc = maxargc;
    /* if(minargc > 0) self->cf.sigs = list_allocptrwithcapa(minargc); */
    self->cf.sigs = list_allocptrwithcapa(minargc);
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
    self->type = type;
    return self;
}

/* update */
lk_gfunc_t *lk_func_combine(lk_func_t *self, lk_func_t *other) {
    lk_vm_t *vm = LK_VM(self);
    list_t *funcs;
    lk_kfunc_t *func;
    lk_sig_t *arg;
    int func_i, func_c, arg_i;
    int dist, other_dist = 0;
    if(!LK_OBJECT_ISGFUNC(LK_O(self))) {
        lk_gfunc_t *gf = lk_gfunc_new(vm);
        list_pushptr(gf->funcs, self);
        self = LK_FUNC(gf);
    }
    funcs = LK_GFUNC(self)->funcs;
    if(other->cf.sigs != NULL) {
        for(arg_i = 0; arg_i < LIST_COUNT(other->cf.sigs); arg_i ++) {
            arg = LIST_ATPTR(other->cf.sigs, arg_i);
            other_dist += lk_object_isa(arg->type, vm->t_object);
        }
    }
    func_c = LIST_COUNT(funcs);
    for(func_i = 0; func_i < func_c; func_i ++) {
        func = LIST_ATPTR(funcs, func_i);
        dist = 0;
        if(func->cf.sigs != NULL) {
            for(arg_i = 0; arg_i < LIST_COUNT(func->cf.sigs); arg_i ++) {
                arg = LIST_ATPTR(func->cf.sigs, arg_i);
                dist += lk_object_isa(arg->type, vm->t_object);
            }
        }
        if(other_dist >= dist) break;
    }
    if(func_i < func_c) list_insertptr(funcs, func_i, other);
    else list_pushptr(funcs, other);
    return LK_GFUNC(self);
}
lk_func_t *lk_func_match(lk_func_t *self, lk_frame_t *args, lk_object_t *recv) {
    lk_vm_t *vm = LK_VM(self);
    list_t *funcs, *stack, *sigs;
    int funci, funcc, argi = 0, argc;
    lk_func_t *curr;
    lk_sig_t *sig;
    lk_object_t *arg;
    if(LK_OBJECT_ISGFUNC(LK_O(self))) {
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
                if(sig == NULL || sig->type == NULL) goto bindarg;
                if(LK_OBJECT_ISTYPE(arg, sig->type)) goto bindarg;
                goto nextfunc;
                bindarg:
                if(sig->name != NULL) lk_object_setslot(sig->isself
                ? recv : LK_O(args), LK_O(sig->name), sig->type, arg);
            }
        }
        if(curr->cf.rest != NULL) {
            sig = curr->cf.rest;
            if(sig != NULL && sig->name != NULL) {
                arg = LK_O(LIST_ISINIT(stack)
                ? lk_list_newfromlist(vm, stack) : lk_list_new(vm));
                list_offset(LIST(arg), argi);
                lk_object_setslot(sig->isself
                ? recv : LK_O(args), LK_O(sig->name), sig->type, arg);
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
    list_t *sigs = self->cf.sigs;
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
            if(list_cmpcstr(LIST(name), "define!") == 0) {
                argdef = argdef->next;
                if(argdef != NULL && argdef->type == LK_INSTRTYPE_APPLY) {
                    typeinstr = LK_INSTR(argdef->v);
                    name = LK_STRING(typeinstr->v);
                    typeinstr = typeinstr->next;
                    slot = lk_object_getdef(LK_O(VM->currframe), typeinstr->v);
                    if(slot != NULL) {
                        type = lk_object_getslot(LK_O(VM->currframe), slot);
                    } else {
                        printf("Invalid sig, invalid type\n");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    printf("Invalid sig, expected type\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                type = VM->t_object;
            }
            lk_object_addref(LK_O(self), LK_O(
            sig = lk_sig_new(VM, name, type)));
            sig->isself = isself;
            if(sigs == NULL) sigs = self->cf.sigs = list_allocptr();
            if(LIST_COUNT(LIST(name)) > 3
            && list_getuchar(LIST(name), -1) == '.'
            && list_getuchar(LIST(name), -2) == '.'
            && list_getuchar(LIST(name), -3) == '.') {
                sig->name = name = LK_STRING(lk_object_clone(LK_O(name)));
                list_limit(LIST(name), -3);
                self->cf.minargc = LIST_COUNT(sigs);
                self->cf.maxargc = INT_MAX;
                self->cf.rest = sig;
                return;
            } else {
                list_pushptr(sigs, sig);
            }
        } else {
            printf("Invalid sig, expected name");
            exit(EXIT_FAILURE);
        }
    }
    self->cf.minargc =
    self->cf.maxargc = sigs != NULL ? LIST_COUNT(sigs) : 0;
}
