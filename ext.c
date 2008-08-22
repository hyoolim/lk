#include "ext.h"
#include "object.h"
#include "string.h"
#include <dlfcn.h>
#include <stdarg.h>

/* ext map - types */
static LK_OBJ_DEFFREEFUNC(free_ext) {
    if(LK_EXT(self)->lib != NULL) dlclose(LK_EXT(self)->lib);
}
LK_LIB_DEFINECFUNC(init_ext_str_str) {
    const char *libpath = darray_toCString(DARRAY(ARG(0)));
    void *lib = dlopen(libpath, RTLD_NOW);
    if(lib != NULL) {
        const char *initname = darray_toCString(DARRAY(ARG(1)));
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
LK_LIB_DEFINEINIT(lk_library_extinit) {
    lk_object_t *str = vm->t_string;
    lk_object_t *ext = lk_object_allocWithSize(vm->t_object, sizeof(lk_library_t));
    lk_object_setfreefunc(ext, free_ext);
    lk_lib_setGlobal("Extension", ext);
    lk_lib_setCFunc(ext, "init!", init_ext_str_str, str, str, NULL);
}

/* update */
void lk_lib_setObject(lk_object_t *parent, const char *k, lk_object_t *v) {
    lk_vm_t *vm = LK_VM(parent);
    lk_string_t *k_kc = lk_string_newFromCString(vm, k);
    lk_object_setslot(parent, LK_OBJ(k_kc), vm->t_object, v);
    /*
    lk_object_setslot(v, LK_OBJ(vm->str_type), vm->t_string, LK_OBJ(k_kc));
    */
}
void lk_lib_setGlobal(const char *k, lk_object_t *v) {
    lk_lib_setObject(LK_OBJ(LK_VM(v)->global), k, v);
}
void lk_lib_setCField(lk_object_t *self, const char *k, lk_object_t *t,
                   size_t offset) {
    lk_vm_t *vm = LK_VM(self);
    lk_string_t *k_kc = lk_string_newFromCString(vm, k);
    struct lk_slot *slot = lk_object_setslot(
    LK_OBJ(self), LK_OBJ(k_kc), t, vm->t_nil);
    assert(offset >= sizeof(struct lk_common));
    LK_SLOT_SETTYPE(slot, LK_SLOTTYPE_CFIELDLKOBJ);
    slot->value.coffset = offset;
}
void lk_lib_setCFunc(lk_object_t *obj, const char *k, lk_cfuncfunc_t *func, ...) {
    lk_vm_t *vm = LK_VM(obj);
    va_list args;
    int i;
    lk_object_t *a;
    lk_string_t *lkk = lk_string_newFromCString(vm, k);
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
