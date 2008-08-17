#include "ext.h"
#include "obj.h"
#include "string.h"
#include <dlfcn.h>
#include <stdarg.h>

/* ext map - types */
static LK_OBJ_DEFFREEFUNC(free__ext) {
    if(LK_EXT(self)->lib != NULL) dlclose(LK_EXT(self)->lib);
}
LK_LIBRARY_DEFINECFUNCTION(init__ext_str_str) {
    const char *libpath = darray_toCString(LIST(ARG(0)));
    void *lib = dlopen(libpath, RTLD_NOW);
    if(lib != NULL) {
        const char *initname = darray_toCString(LIST(ARG(1)));
        union { void *p; lk_libraryinitfunc_t *f; } initfunc;
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
LK_EXT_DEFINIT(lk_library_extinit) {
    lk_object_t *str = vm->t_string;
    lk_object_t *ext = lk_object_allocwithsize(vm->t_obj, sizeof(lk_library_t));
    lk_object_setfreefunc(ext, free__ext);
    lk_library_setGlobal("Extension", ext);
    lk_library_setCFunction(ext, "init", init__ext_str_str, str, str, NULL);
}

/* update */
void lk_library_set(lk_object_t *parent, const char *k, lk_object_t *v) {
    lk_vm_t *vm = LK_VM(parent);
    lk_string_t *k_kc = lk_string_newfromcstr(vm, k);
    lk_object_setslot(parent, LK_OBJ(k_kc), vm->t_obj, v);
    /*
    lk_object_setslot(v, LK_OBJ(vm->str_type), vm->t_string, LK_OBJ(k_kc));
    */
}
void lk_library_setGlobal(const char *k, lk_object_t *v) {
    lk_library_set(LK_OBJ(LK_VM(v)->global), k, v);
}
void lk_library_cfield(lk_object_t *self, const char *k, lk_object_t *t,
                   size_t offset) {
    lk_vm_t *vm = LK_VM(self);
    lk_string_t *k_kc = lk_string_newfromcstr(vm, k);
    struct lk_slot *slot = lk_object_setslot(
    LK_OBJ(self), LK_OBJ(k_kc), t, vm->t_nil);
    assert(offset >= sizeof(struct lk_common));
    LK_SLOT_SETTYPE(slot, LK_SLOTTYPE_CFIELDLKOBJ);
    slot->value.coffset = offset;
}
void lk_library_setCFunction(lk_object_t *obj, const char *k, lk_cfuncfunc_t *func, ...) {
    lk_vm_t *vm = LK_VM(obj);
    va_list args;
    int i;
    lk_object_t *a;
    lk_string_t *lkk = lk_string_newfromcstr(vm, k);
    lk_cfunc_t *cfunc = lk_cfunc_new(vm, func, 0, 0);
    struct lk_slot *slot = lk_object_getslot(obj, LK_OBJ(lkk));
    if(slot != NULL) {
        lk_object_t *old = lk_object_getvaluefromslot(obj, slot);
        if(LK_OBJ_ISFUNC(old)) {
            old = LK_OBJ(lk_func_combine(LK_FUNC(old), LK_FUNC(cfunc)));
        }
        lk_object_setvalueonslot(obj, slot, old);
    } else {
        slot = lk_object_setslot(obj, LK_OBJ(lkk), vm->t_func, LK_OBJ(cfunc));
    }
    LK_SLOT_SETOPTION(slot, LK_SLOTOPTION_AUTOSEND);
    va_start(args, func);
    for(i = 0; ; i ++) {
        a = va_arg(args, lk_object_t *);
        if(a == NULL) {
            cfunc->cf.minargc = cfunc->cf.maxargc = i;
            break;
        }
        if(a == (lk_object_t *)-1) {
            cfunc->cf.maxargc = INT_MAX;
            break;
        }
        darray_setptr(cfunc->cf.sigs, i, lk_sig_new(vm, NULL, a));
    }
    va_end(args);
}
