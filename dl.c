#include "dl.h"
#include <dlfcn.h>

/* type */
static void free_dl(lk_obj_t *self) {
    if(LK_DL(self)->dl != NULL) {
        dlclose(LK_DL(self)->dl);
    }
}
lk
static void init_dl_str_str(lk_obj_t *self, lk_scope_t *local) {
    const char *libpath = darray_tocstr(DARRAY(ARG(0)));
    void *lib = dlopen(libpath, RTLD_NOW);
    if(lib != NULL) {
        const char *initname = darray_tocstr(DARRAY(ARG(1)));
        union { void *p; lk_dlinitfunc_t *f; } initfunc;
        initfunc.p = dlsym(lib, initname);
        LK_DL(self)->lib = lib;
        if(initfunc.f != NULL) initfunc.f(VM);
        else {
            printf("dlsym: %s\n", dlerror());
        }
    } else {
        printf("dlopen: %s\n", dlerror());
    }
    RETURN(self);
}
void lk_dl_libinit(lk_vm_t *vm) {
    lk_obj_t *str = vm->t_str;
    lk_obj_t *dl = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_dl_t));
    lk_obj_setfreefunc(dl, free_dl);
    lk_global_set("DynamicLibrary", dl);
    lk_obj_set_cfunc_lk(dl, "init!", init_dl_str_str, str, str, NULL);
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
