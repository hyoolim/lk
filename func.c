#include "lib.h"

/* ext map - types */
/* */
static void alloc_f(lk_obj_t *self, lk_obj_t *parent) {
    LK_FUNC(self)->cf.sigdef = LK_FUNC(parent)->cf.sigdef;
    LK_FUNC(self)->cf.minargc = LK_FUNC(parent)->cf.minargc;
    LK_FUNC(self)->cf.maxargc = LK_FUNC(parent)->cf.maxargc;
    if(LK_FUNC(self)->cf.sigs != NULL
    ) LK_FUNC(self)->cf.sigs = darray_clone(LK_FUNC(parent)->cf.sigs);
    LK_FUNC(self)->cf.rest = LK_FUNC(parent)->cf.rest;
    LK_FUNC(self)->cf.opts = LK_FUNC(parent)->cf.opts;
}
static LK_OBJ_DEFMARKFUNC(mark_f) {
    mark(LK_OBJ(LK_FUNC(self)->cf.sigdef));
    if(LK_FUNC(self)->cf.sigs != NULL
    ) DARRAY_EACHPTR(LK_FUNC(self)->cf.sigs, i, v, mark(v));
    mark(LK_OBJ(LK_FUNC(self)->cf.rest));
}
static void free_f(lk_obj_t *self) {
    if(LK_FUNC(self)->cf.sigs != NULL) darray_free(LK_FUNC(self)->cf.sigs);
}

/* */
static void alloc_cf(lk_obj_t *self, lk_obj_t *parent) {
    alloc_f(self, parent);
    LK_CFUNC(self)->cc = LK_CFUNC(parent)->cc;
    LK_CFUNC(self)->cfunc = LK_CFUNC(parent)->cfunc;
}

/* */
static void alloc_gf(lk_obj_t *self, lk_obj_t *parent) {
    alloc_f(self, parent);
    LK_GFUNC(self)->funcs = darray_clone(LK_GFUNC(parent)->funcs);
}
static LK_OBJ_DEFMARKFUNC(mark_gf) {
    mark_f(self, mark);
    DARRAY_EACHPTR(LK_GFUNC(self)->funcs, i, v, mark(v));
}
static void free_gf(lk_obj_t *self) {
    free_f(self);
    darray_free(LK_GFUNC(self)->funcs);
}

/* */
static void alloc_kf(lk_obj_t *self, lk_obj_t *parent) {
    alloc_f(self, parent);
    LK_KFUNC(self)->scope = LK_KFUNC(parent)->scope;
    LK_KFUNC(self)->first = LK_KFUNC(parent)->first;
}
static LK_OBJ_DEFMARKFUNC(mark_kf) {
    mark_f(self, mark);
    mark(LK_OBJ(LK_KFUNC(self)->scope));
    mark(LK_OBJ(LK_KFUNC(self)->first));
}

/* */
static void alloc_sig(lk_obj_t *self, lk_obj_t *parent) {
    LK_SIG(self)->name = LK_SIG(parent)->name;
    LK_SIG(self)->check = LK_SIG(parent)->check;
    LK_SIG(self)->isself = LK_SIG(parent)->isself;
}
static LK_OBJ_DEFMARKFUNC(mark_sig) {
    mark(LK_OBJ(LK_SIG(self)->name));
    mark(LK_OBJ(LK_SIG(self)->check));
}
void lk_func_typeinit(lk_vm_t *vm) {
    /* */
    vm->t_func = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_func_t));
    lk_obj_setallocfunc(vm->t_func, alloc_f);
    lk_obj_setmarkfunc(vm->t_func, mark_f);
    lk_obj_setfreefunc(vm->t_func, free_f);
    /* */
    vm->t_cfunc = lk_obj_alloc_withsize(vm->t_func, sizeof(lk_cfunc_t));
    lk_obj_setallocfunc(vm->t_cfunc, alloc_cf);
    /* */
    vm->t_gfunc = lk_obj_alloc_withsize(vm->t_func, sizeof(lk_gfunc_t));
    LK_GFUNC(vm->t_gfunc)->funcs = darray_alloc(sizeof(lk_func_t *), 4);
    lk_obj_setallocfunc(vm->t_gfunc, alloc_gf);
    lk_obj_setmarkfunc(vm->t_gfunc, mark_gf);
    lk_obj_setfreefunc(vm->t_gfunc, free_gf);
    /* */
    vm->t_kfunc = lk_obj_alloc_withsize(vm->t_func, sizeof(lk_kfunc_t));
    lk_obj_setallocfunc(vm->t_kfunc, alloc_kf);
    lk_obj_setmarkfunc(vm->t_kfunc, mark_kf);
    /* */
    vm->t_sig = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_sig_t));
    lk_obj_setallocfunc(vm->t_sig, alloc_sig);
    lk_obj_setmarkfunc(vm->t_sig, mark_sig);
}

/* ext map - funcs */
static void add_f_f(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_func_combine(LK_FUNC(self), LK_FUNC(ARG(0))));
}
static void addB_f_f(lk_obj_t *self, lk_scope_t *local) {
    if(local->caller == NULL && local->caller->lastslot == NULL) {
        lk_vm_raisecstr(VM, "Cannot add to the function without a slot");
    } else {
        lk_gfunc_t *new = lk_func_combine(LK_FUNC(self), LK_FUNC(ARG(0)));
        lk_obj_setvalueonslot(self, local->caller->lastslot, LK_OBJ(new));
        RETURN(new);
    }
}
static void minimum_argument_size_f(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, LK_FUNC(self)->cf.minargc));
}
static void maximum_argument_size_f(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, LK_FUNC(self)->cf.maxargc));
}
static void signature_f(lk_obj_t *self, lk_scope_t *local) {
    darray_t *sigs = LK_FUNC(self)->cf.sigs;
    RETURN(sigs != NULL ? lk_list_new_fromdarray(VM, sigs) : lk_list_new(VM));
}
static void last_argument_f(lk_obj_t *self, lk_scope_t *local) {
    lk_sig_t *last = LK_FUNC(self)->cf.rest;
    RETURN(last != NULL ? LK_OBJ(last) : NIL);
}
static void functions_gf(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_list_new_fromdarray(VM, LK_GFUNC(self)->funcs));
}
static void name_sig(lk_obj_t *self, lk_scope_t *local) {
    lk_str_t *name = LK_SIG(self)->name;
    RETURN(name != NULL ? name : lk_str_new_fromcstr(VM, "_"));
}
static void check_sig(lk_obj_t *self, lk_scope_t *local) {
    lk_obj_t *type = LK_SIG(self)->check;
    RETURN(type != NULL ? type : VM->t_obj);
}
void lk_func_libinit(lk_vm_t *vm) {
    lk_obj_t *f = vm->t_func, *sig = vm->t_sig;
    /* */
    lk_global_set("Function", vm->t_func);
    lk_obj_set_cfunc_lk(f, "+", add_f_f, f, NULL);
    lk_obj_set_cfunc_lk(f, "+=", addB_f_f, f, NULL);
    lk_obj_set_cfield(f, "doc", f, offsetof(lk_func_t, cf.doc));
    lk_obj_set_cfunc_lk(f, "minimum_argument_size", minimum_argument_size_f, NULL);
    lk_obj_set_cfunc_lk(f, "maximum_argument_size", maximum_argument_size_f, NULL);
    lk_obj_set_cfunc_lk(f, "signature", signature_f, NULL);
    lk_obj_set_cfunc_lk(f, "last_argument", last_argument_f, NULL);
    /* */
    lk_global_set("CFunction", vm->t_cfunc);
    /* */
    lk_global_set("GenericFunction", vm->t_gfunc);
    lk_obj_set_cfunc_lk(vm->t_gfunc, "functions", functions_gf, NULL);
    /* */
    lk_global_set("KineticFunction", vm->t_kfunc);
    /* */
    lk_global_set("Signature", vm->t_sig);
    lk_obj_set_cfunc_lk(sig, "name", name_sig, NULL);
    lk_obj_set_cfunc_lk(sig, "check", check_sig, NULL);
}

/* new */
lk_cfunc_t *lk_cfunc_new(lk_vm_t *vm, lk_cfunc_lk_t *cfunc, int minargc, int maxargc) {
    lk_cfunc_t *self = LK_CFUNC(lk_obj_alloc(vm->t_cfunc));
    self->cfunc.lk = cfunc;
    self->cf.minargc = minargc;
    self->cf.maxargc = maxargc;
    /* if(minargc > 0) self->cf.sigs = darray_ptr_alloc_withcap(minargc); */
    self->cf.sigs = darray_ptr_alloc_withcap(minargc);
    return self;
}
lk_kfunc_t *lk_kfunc_new(lk_vm_t *vm, lk_scope_t *scope, lk_instr_t *first) {
    lk_kfunc_t *self = LK_KFUNC(lk_obj_alloc(vm->t_kfunc));
    self->scope = scope;
    self->first = first;
    self->cf.maxargc = INT_MAX;
    return self;
}
lk_gfunc_t *lk_gfunc_new(lk_vm_t *vm) {
    lk_gfunc_t *self = LK_GFUNC(lk_obj_alloc(vm->t_gfunc));
    return self;
}
lk_sig_t *lk_sig_new(lk_vm_t *vm, lk_str_t *name, lk_obj_t *type) {
    lk_sig_t *self = LK_SIG(lk_obj_alloc(vm->t_sig));
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
        darray_ptr_push(gf->funcs, self);
        if(CHKOPT(LK_FUNC(self)->cf.opts, LK_FUNCOASSIGNED)) {
            SETOPT(LK_FUNC(gf)->cf.opts, LK_FUNCOASSIGNED);
        }
        self = LK_FUNC(gf);
    }
    funcs = LK_GFUNC(self)->funcs;
    if(other->cf.sigs != NULL) {
        for(arg_i = 0; arg_i < DARRAY_COUNT(other->cf.sigs); arg_i ++) {
            arg = DARRAY_ATPTR(other->cf.sigs, arg_i);
            other_dist += lk_obj_isa(arg->check, vm->t_obj);
        }
    }
    func_c = DARRAY_COUNT(funcs);
    for(func_i = 0; func_i < func_c; func_i ++) {
        func = DARRAY_ATPTR(funcs, func_i);
        dist = 0;
        if(func->cf.sigs != NULL) {
            for(arg_i = 0; arg_i < DARRAY_COUNT(func->cf.sigs); arg_i ++) {
                arg = DARRAY_ATPTR(func->cf.sigs, arg_i);
                dist += lk_obj_isa(arg->check, vm->t_obj);
            }
        }
        if(other_dist >= dist) break;
    }
    if(func_i < func_c) darray_ptr_insert(funcs, func_i, other);
    else darray_ptr_push(funcs, other);
    return LK_GFUNC(self);
}
lk_func_t *lk_func_match(lk_func_t *self, lk_scope_t *args, lk_obj_t *recv) {
    lk_vm_t *vm = LK_VM(self);
    darray_t *funcs, *stack, *sigs;
    int funci, funcc, argi = 0, argc;
    lk_func_t *curr;
    lk_sig_t *sig;
    lk_obj_t *arg;
    if(LK_OBJ_ISGFUNC(LK_OBJ(self))) {
        funcs = LK_GFUNC(self)->funcs;
        funci = 0; funcc = DARRAY_COUNT(funcs);
        curr = DARRAY_ATPTR(funcs, funci);
    } else {
        funcs = NULL;
        funci = 0; funcc = 1;
        curr = self;
    }
    findfunc:
    stack = &args->stack;
    argc = DARRAY_COUNT(stack);
    if(curr->cf.minargc <= argc && argc <= curr->cf.maxargc) {
        sigs = curr->cf.sigs;
        if(sigs != NULL) {
            for(; argi < DARRAY_COUNT(sigs); argi ++) {
                sig = DARRAY_ATPTR(sigs, argi);
                arg = DARRAY_ATPTR(stack, argi);
                if(sig == NULL || sig->check == NULL) goto bindarg;
                if(LK_OBJ_ISTYPE(arg, sig->check)) goto bindarg;
                goto nextfunc;
                bindarg:
                if(sig->name != NULL) lk_obj_setslot(sig->isself
                ? recv : LK_OBJ(args), LK_OBJ(sig->name), sig->check, arg);
            }
        }
        if(curr->cf.rest != NULL) {
            sig = curr->cf.rest;
            if(sig != NULL && sig->name != NULL) {
                arg = LK_OBJ(DARRAY_ISINIT(stack)
                ? lk_list_new_fromdarray(vm, stack) : lk_list_new(vm));
                darray_offset(DARRAY(arg), argi);
                lk_obj_setslot(sig->isself
                ? recv : LK_OBJ(args), LK_OBJ(sig->name), sig->check, arg);
            }
        }
        return curr;
    }
    nextfunc:
    if(++ funci >= funcc) return NULL;
    curr = DARRAY_ATPTR(funcs, funci);
    goto findfunc;
}
void lk_kfunc_updatesig(lk_kfunc_t *self) {
    lk_instr_t *typeinstr, *argdef = self->cf.sigdef;
    int isself;
    struct lk_slot *slot;
    darray_t *sigs = self->cf.sigs;
    lk_str_t *name;
    lk_obj_t *type;
    lk_sig_t *sig;
    if(argdef != NULL && argdef->type == LK_INSTRTYPE_STRING) {
        self->cf.doc = LK_STRING(argdef->v);
        argdef = argdef->next;
    }
    for(; argdef != NULL; argdef = argdef->next) {
        if((isself = argdef->type == LK_INSTRTYPE_SELFMSG)
        || argdef->type == LK_INSTRTYPE_SCOPEMSG) {
            name = LK_STRING(argdef->v);
            if(darray_str_cmp_cstr(DARRAY(name), ":") == 0) {
                argdef = argdef->next;
                if(argdef != NULL && argdef->type == LK_INSTRTYPE_APPLY) {
                    typeinstr = LK_INSTR(argdef->v);
                    name = LK_STRING(typeinstr->v);
                    typeinstr = typeinstr->next;
                    slot = lk_obj_getslotfromany(LK_OBJ(VM->currscope), typeinstr->v);
                    if(slot != NULL) {
                        type = lk_obj_getvaluefromslot(LK_OBJ(VM->currscope), slot);
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
            lk_obj_addref(LK_OBJ(self), LK_OBJ(
            sig = lk_sig_new(VM, name, type)));
            sig->isself = isself;
            if(sigs == NULL) sigs = self->cf.sigs = darray_ptr_alloc();
            if(DARRAY_COUNT(DARRAY(name)) > 3
            && darray_str_get(DARRAY(name), -1) == '.'
            && darray_str_get(DARRAY(name), -2) == '.'
            && darray_str_get(DARRAY(name), -3) == '.') {
                sig->name = name = LK_STRING(lk_obj_clone(LK_OBJ(name)));
                darray_limit(DARRAY(name), -3);
                self->cf.minargc = DARRAY_COUNT(sigs);
                self->cf.maxargc = INT_MAX;
                self->cf.rest = sig;
                return;
            } else {
                darray_ptr_push(sigs, sig);
            }
        } else {
            printf("Invalid sig, expected name");
            exit(EXIT_FAILURE);
        }
    }
    self->cf.minargc =
    self->cf.maxargc = sigs != NULL ? DARRAY_COUNT(sigs) : 0;
}
