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
    const char *libpath = Sequence_tocstr(LIST(ARG(0)));
    void *lib = dlopen(libpath, RTLD_NOW);
    if(lib != NULL) {
        const char *initname = Sequence_tocstr(LIST(ARG(1)));
        union { void *p; lk_Libraryinitfunc_t *f; } initfunc;
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
LK_EXT_DEFINIT(lk_Library_extinit) {
    lk_Object_t *str = vm->t_string;
    lk_Object_t *ext = lk_Object_allocwithsize(vm->t_obj, sizeof(lk_Library_t));
    lk_Object_setfreefunc(ext, free__ext);
    lk_Library_setGlobal("Extension", ext);
    lk_Library_setCFunction(ext, "init", init__ext_str_str, str, str, NULL);
}

/* update */
void lk_Library_set(lk_Object_t *parent, const char *k, lk_Object_t *v) {
    lk_Vm_t *vm = LK_VM(parent);
    lk_String_t *k_kc = lk_String_newfromcstr(vm, k);
    lk_Object_setslot(parent, LK_OBJ(k_kc), vm->t_obj, v);
    /*
    lk_Object_setslot(v, LK_OBJ(vm->str_type), vm->t_string, LK_OBJ(k_kc));
    */
}
void lk_Library_setGlobal(const char *k, lk_Object_t *v) {
    lk_Library_set(LK_OBJ(LK_VM(v)->global), k, v);
}
void lk_Library_cfield(lk_Object_t *self, const char *k, lk_Object_t *t,
                   size_t offset) {
    lk_Vm_t *vm = LK_VM(self);
    lk_String_t *k_kc = lk_String_newfromcstr(vm, k);
    struct lk_Slot *slot = lk_Object_setslot(
    LK_OBJ(self), LK_OBJ(k_kc), t, vm->t_nil);
    assert(offset >= sizeof(struct lk_Common));
    LK_SLOT_SETTYPE(slot, LK_SLOTTYPE_CFIELDLKOBJ);
    slot->value.coffset = offset;
}
void lk_Library_setCFunction(lk_Object_t *obj, const char *k, lk_Cfuncfunc_t *func, ...) {
    lk_Vm_t *vm = LK_VM(obj);
    va_list args;
    int i;
    lk_Object_t *a;
    lk_String_t *lkk = lk_String_newfromcstr(vm, k);
    lk_Cfunc_t *cfunc = lk_Cfunc_new(vm, func, 0, 0);
    struct lk_Slot *slot = lk_Object_getslot(obj, LK_OBJ(lkk));
    if(slot != NULL) {
        lk_Object_t *old = lk_Object_getvaluefromslot(obj, slot);
        if(LK_OBJ_ISFUNC(old)) {
            old = LK_OBJ(lk_Func_combine(LK_FUNC(old), LK_FUNC(cfunc)));
        }
        lk_Object_setvalueonslot(obj, slot, old);
    } else {
        slot = lk_Object_setslot(obj, LK_OBJ(lkk), vm->t_func, LK_OBJ(cfunc));
    }
    LK_SLOT_SETOPTION(slot, LK_SLOTOPTION_AUTOSEND);
    va_start(args, func);
    for(i = 0; ; i ++) {
        a = va_arg(args, lk_Object_t *);
        if(a == NULL) {
            cfunc->cf.minargc = cfunc->cf.maxargc = i;
            break;
        }
        if(a == (lk_Object_t *)-1) {
            cfunc->cf.maxargc = INT_MAX;
            break;
        }
        Sequence_setptr(cfunc->cf.sigs, i, lk_Sig_new(vm, NULL, a));
    }
    va_end(args);
}
