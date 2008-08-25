#include "ext.h"
#include "object.h"
#include "string.h"
#include <dlfcn.h>
#include <stdarg.h>

/* ext map - types */
static void free_ext(lk_object_t *self) {
    if(LK_EXT(self)->lib != NULL) dlclose(LK_EXT(self)->lib);
}
static void init_ext_str_str(lk_object_t *self, lk_scope_t *local) {
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
void lk_library_extinit(lk_vm_t *vm) {
    lk_object_t *str = vm->t_string;
    lk_object_t *ext = lk_object_allocWithSize(vm->t_object, sizeof(lk_library_t));
    lk_object_setfreefunc(ext, free_ext);
    lk_lib_setGlobal("Extension", ext);
    lk_object_set_cfunc_lk(ext, "init!", init_ext_str_str, str, str, NULL);
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
