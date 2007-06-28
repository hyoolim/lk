#include "ext.h"
#include "obj.h"
#include "string.h"
#include <dlfcn.h>
#include <stdarg.h>

/* ext map - types */
static LK_OBJ_DEFFREEFUNC(free__ext) {
    if(LK_EXT(self)->lib != NULL) dlclose(LK_EXT(self)->lib);
}
static LK_EXT_DEFCFUNC(init__ext_str_str) {
    const char *libpath = list_tocstr(LIST(ARG(0)));
    void *lib = dlopen(libpath, RTLD_NOW);
    if(lib != NULL) {
        const char *initname = list_tocstr(LIST(ARG(1)));
        union { void *p; lk_extinitfunc_t *f; } initfunc;
        initfunc.p = dlsym(lib, initname);
        LK_EXT(self)->lib = lib;
        if(initfunc.f != NULL) initfunc.f(VM);
        else {
            printf("dlsym: %s\n", dlerror());
        }
    } else {
        printf("dlopen: %s\n", dlerror());
    }
    RETURN(self);
}
LK_EXT_DEFINIT(lk_ext_extinit) {
    lk_obj_t *str = vm->t_string;
    lk_obj_t *ext = lk_obj_allocwithsize(vm->t_obj, sizeof(lk_ext_t));
    lk_obj_setfreefunc(ext, free__ext);
    lk_ext_global("Extension", ext);
    lk_ext_cfunc(ext, "init", init__ext_str_str, str, str, NULL);
}

/* update */
void lk_ext_set(lk_obj_t *proto, const char *k, lk_obj_t *v) {
    lk_vm_t *vm = LK_VM(proto);
    lk_string_t *type = vm->str_type;
    lk_string_t *k_kc = lk_string_newfromcstr(vm, k);
    lk_obj_setslot(proto, LK_OBJ(k_kc), vm->t_obj, v);
    /*
    lk_obj_setslot(v, LK_OBJ(type), vm->t_string, LK_OBJ(k_kc));
    */
}
void lk_ext_global(const char *k, lk_obj_t *v) {
    lk_ext_set(LK_OBJ(LK_VM(v)->global), k, v);
}
void lk_ext_cfield(lk_obj_t *self, const char *k, lk_obj_t *t,
                   size_t offset) {
    lk_vm_t *vm = LK_VM(self);
    lk_string_t *k_kc = lk_string_newfromcstr(vm, k);
    struct lk_slot *slot = lk_obj_setslot(
    LK_OBJ(self), LK_OBJ(k_kc), t, vm->t_unknown);
    assert(offset >= sizeof(struct lk_common));
    LK_SLOT_SETTYPE(slot, LK_SLOTTYPE_CFIELDLKOBJ);
    slot->value.coffset = offset;
}
void lk_ext_cfunc(lk_obj_t *obj, const char *k, lk_cfuncfunc_t *func, ...) {
    lk_vm_t *vm = LK_VM(obj);
    va_list args;
    int i;
    lk_obj_t *a;
    lk_string_t *lkk = lk_string_newfromcstr(vm, k);
    lk_cfunc_t *cfunc = lk_cfunc_new(vm, func, 0, 0);
    struct lk_slot *slot = lk_obj_getslot(obj, LK_OBJ(lkk));
    if(slot != NULL) {
        lk_obj_t *old = lk_obj_getvaluefromslot(obj, slot);
        if(LK_OBJ_ISFUNC(old)) {
            old = LK_OBJ(lk_func_combine(LK_FUNC(old), LK_FUNC(cfunc)));
        }
        lk_obj_setvalueonslot(obj, slot, old);
    } else {
        slot = lk_obj_setslot(obj, LK_OBJ(lkk), vm->t_func, LK_OBJ(cfunc));
    }
    LK_SLOT_SETOPTION(slot, LK_SLOTOPTION_AUTOSEND);
    va_start(args, func);
    for(i = 0; ; i ++) {
        a = va_arg(args, lk_obj_t *);
        if(a == NULL) {
            cfunc->cf.minargc = cfunc->cf.maxargc = i;
            break;
        }
        if(a == (lk_obj_t *)-1) {
            cfunc->cf.maxargc = INT_MAX;
            break;
        }
        list_setptr(cfunc->cf.sigs, i, lk_sig_new(vm, NULL, a));
    }
    va_end(args);
}
