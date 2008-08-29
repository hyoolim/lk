#include "lib.h"
#include <dlfcn.h>

/* type */
static void free_dl(lk_obj_t *self) {
    if(LK_DL(self)->dl != NULL) {
        dlclose(LK_DL(self)->dl);
        LK_DL(self)->dl = NULL;
    }
}
void lk_dl_typeinit(lk_vm_t *vm) {
    vm->t_dl = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_dl_t));
    lk_obj_setfreefunc(vm->t_dl, free_dl);
}

/* new */
void lk_dl_init_withpath_andfunc(lk_dl_t *self, lk_str_t *path, lk_str_t *funcname) {
    void *dl = dlopen(darray_tocstr(DARRAY(path)), RTLD_NOW);
    if(dl != NULL) {
        union { void *p; void (*f)(lk_vm_t *vm); } func;
        self->dl = dl;
        func.p = dlsym(dl, darray_tocstr(DARRAY(funcname)));
        if(func.f != NULL) {
            func.f(VM);
        } else {
            printf("dlsym: %s\n", dlerror());
        }
    } else {
        printf("dlopen: %s\n", dlerror());
    }
}

/* bind all c funcs to lk equiv */
void lk_dl_libinit(lk_vm_t *vm) {
    lk_obj_t *dl = vm->t_dl, *str = vm->t_str;
    lk_global_set("DynamicLibrary", dl);

    /* new */
    lk_obj_set_cfunc_cvoid(dl, "init!", lk_dl_init_withpath_andfunc, str, str, NULL);
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
