#include "lib.h"

// type
static void free_cptr(lk_obj_t *self) {
    lk_cptr_t *cptr = LK_CPTR(self);

    if (cptr->free_func != NULL && cptr->ptr != NULL)
        cptr->free_func(cptr->ptr);
}

void lk_cptr_type_init(lk_vm_t *vm) {
    vm->t_cptr = lk_obj_alloc_type(vm->t_obj, sizeof(lk_cptr_t));
    lk_obj_set_free_func(vm->t_cptr, free_cptr);
}

// new
lk_cptr_t *lk_cptr_new(lk_vm_t *vm, void *ptr, lk_cptr_free_func_t *free_func) {
    lk_cptr_t *self = LK_CPTR(lk_obj_alloc(vm->t_cptr));

    self->ptr = ptr;
    self->free_func = free_func;
    return self;
}

// lk methods
static void Dnull_q_cptr(lk_obj_t *self, lk_scope_t *local) {
    RETURN(LK_CPTR(self)->ptr == NULL ? TRUE : FALSE);
}

static void Daddress_cptr(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, (double)(uintptr_t)LK_CPTR(self)->ptr));
}

// bind all c funcs to lk equiv
void lk_cptr_lib_init(lk_vm_t *vm) {
    lk_obj_t *cptr = vm->t_cptr;

    lk_global_set("CPointer", cptr);

    lk_obj_set_cfunc_lk(cptr, "null?", Dnull_q_cptr, NULL);
    lk_obj_set_cfunc_lk(cptr, "address", Daddress_cptr, NULL);
}
