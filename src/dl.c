#include "lib.h"
#include <dlfcn.h>

// cfield offsets are relative to lk_obj_t*, so lk_common must be at offset 0
_Static_assert(offsetof(lk_obj_t, o) == 0, "lk_common must be first member of lk_obj_t for cfield offsets");

// type
static void close_dl(void *ptr) {
    dlclose(ptr);
}

void lk_dl_type_init(lk_vm_t *vm) {
    vm->t_dl = lk_obj_alloc_type(vm->t_obj, sizeof(lk_obj_t));
}

// new
void lk_dl_init_with_path_and_func(lk_obj_t *self, lk_str_t *path, lk_str_t *funcname) {
    void *dl = dlopen(vec_str_tocstr(VEC(path)), RTLD_NOW);

    if (dl != NULL) {
        union {
            void *p;
            void (*f)(lk_vm_t *vm);
        } func;

        lk_obj_set_slot_by_cstr(self, "dl", NULL, LK_OBJ(lk_cptr_new(VM, dl, close_dl)));
        func.p = dlsym(dl, vec_str_tocstr(VEC(funcname)));

        if (func.f != NULL) {
            func.f(VM);

        } else {
            printf("dlsym: %s\n", dlerror());
        }

    } else {
        printf("dlopen: %s\n", dlerror());
    }
}

// bind all c funcs to lk equiv
void lk_dl_lib_init(lk_vm_t *vm) {
    lk_obj_t *dl = vm->t_dl, *str = vm->t_str;
    lk_global_set("DynamicLibrary", dl);

    // new
    lk_obj_set_cfunc_cvoid(dl, "init!", lk_dl_init_with_path_and_func, str, str, NULL);
}

// update
void lk_object_set(lk_obj_t *parent, const char *k, lk_obj_t *v) {
    lk_vm_t *vm = LK_VM(parent);
    lk_str_t *k_kc = lk_str_new_from_cstr(vm, k);
    lk_obj_setslot(parent, LK_OBJ(k_kc), vm->t_obj, v);
    /*
    lk_obj_setslot(v, LK_OBJ(vm->str_type), vm->t_str, LK_OBJ(k_kc));
    */
}

void lk_global_set(const char *k, lk_obj_t *v) {
    lk_object_set(LK_OBJ(LK_VM(v)->global), k, v);
}

void lk_obj_set_cfield(lk_obj_t *self, const char *k, lk_obj_t *t, size_t offset) {
    lk_vm_t *vm = LK_VM(self);
    lk_str_t *k_kc = lk_str_new_from_cstr(vm, k);
    struct lk_slot *slot = lk_obj_setslot(LK_OBJ(self), LK_OBJ(k_kc), t, vm->t_nil);
    assert(offset >= sizeof(struct lk_common)); // cfield offsets are relative to lk_obj_t*, so must clear lk_common
    LK_SLOT_SETTYPE(slot, LK_SLOTTYPE_CFIELDLKOBJ);
    slot->value.coffset = offset;
}
