#include "ext.h"
#include <dlfcn.h>
#include <stdarg.h>

/* ext map - types */
static void free_ext(lk_obj_t *self) {
    if(LK_EXT(self)->lib != NULL) dlclose(LK_EXT(self)->lib);
}
static void init_ext_str_str(lk_obj_t *self, lk_scope_t *local) {
    const char *libpath = darray_tocstr(DARRAY(ARG(0)));
    void *lib = dlopen(libpath, RTLD_NOW);
    if(lib != NULL) {
        const char *initname = darray_tocstr(DARRAY(ARG(1)));
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
    lk_obj_t *str = vm->t_str;
    lk_obj_t *ext = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_library_t));
    lk_obj_setfreefunc(ext, free_ext);
    lk_global_set("Extension", ext);
    lk_obj_set_cfunc_lk(ext, "init!", init_ext_str_str, str, str, NULL);
}

/* update */
void lk_object_set(lk_obj_t *parent, const char *k, lk_obj_t *v) {
    lk_vm_t *vm = LK_VM(parent);
    lk_str_t *k_kc = lk_str_new_fromcstr(vm, k);
    lk_obj_setslot(parent, LK_OBJ(k_kc), vm->t_obj, v);
    /*
    lk_obj_setslot(v, LK_OBJ(vm->str_type), vm->t_str, LK_OBJ(k_kc));
    */
}
void lk_global_set(const char *k, lk_obj_t *v) {
    lk_object_set(LK_OBJ(LK_VM(v)->global), k, v);
}
void lk_obj_set_cfield(lk_obj_t *self, const char *k, lk_obj_t *t,
                   size_t offset) {
    lk_vm_t *vm = LK_VM(self);
    lk_str_t *k_kc = lk_str_new_fromcstr(vm, k);
    struct lk_slot *slot = lk_obj_setslot(
    LK_OBJ(self), LK_OBJ(k_kc), t, vm->t_nil);
    assert(offset >= sizeof(struct lk_common));
    LK_SLOT_SETTYPE(slot, LK_SLOTTYPE_CFIELDLKOBJ);
    slot->value.coffset = offset;
}
