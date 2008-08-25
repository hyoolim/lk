#include "vector.h"
#include "ext.h"
#include "number.h"

/* ext map - types */
void lk_vector_typeinit(lk_vm_t *vm) {
    vm->t_vector = lk_object_alloc(vm->t_seq);
    darray_fin(DARRAY(vm->t_vector));
    darray_init(DARRAY(vm->t_vector), sizeof(int), 16);
}

/* ext map - funcs */
static void at_vec_number(lk_object_t *self, lk_scope_t *local) {
    int *v = darray_get(DARRAY(self), CSIZE(ARG(0)));
    RETURN(v != NULL ? LK_OBJ(lk_number_new(VM, *v)) : NIL);
}
#define AT(i) (*(int *)LIST_AT(values, *(int *)LIST_AT(indexes, (i))))
#define SWAP(x, y) do { \
    t = *(int *)LIST_AT(indexes, (x)); \
    *(int *)LIST_AT(indexes, (x)) = *(int *)LIST_AT(indexes, (y)); \
    *(int *)LIST_AT(indexes, (y)) = t; \
} while(0)
static void quicksort_hoare(darray_t *values, darray_t *indexes, int low, int hi) {
    if(low < hi) {
        int l = low, h = hi, p = AT(hi), t;
        do {
            while(l < h && AT(l) <= p) l ++;
            while(h > l && AT(h) >= p) h --;
            if(l < h) SWAP(l, h);
        } while(l < h);
        SWAP(l, hi);
        quicksort_hoare(values, indexes, low, l - 1);
        quicksort_hoare(values, indexes, l + 1, hi);
    }
}
static void grade_vec(lk_object_t *self, lk_scope_t *local) {
    lk_vector_t *indexes = LK_VECTOR(lk_object_alloc(VM->t_vector));
    darray_t *sl = DARRAY(self), *il = DARRAY(indexes);
    darray_resize(il, LIST_COUNT(sl));
    LIST_EACH(il, i, v, *(int *)v = i);
    quicksort_hoare(sl, il, 0, LIST_COUNT(il) - 1);
    RETURN(indexes);
}
static void insertB_vec_number_number(lk_object_t *self, lk_scope_t *local) {
    /*
    darray_insert(DARRAY(self), CSIZE(ARG(0)), &CSIZE(ARG(1)));
    */
    RETURN(self);
}
static void removeB_vec_number(lk_object_t *self, lk_scope_t *local) {
    int i = CSIZE(ARG(0));
    int *v = darray_get(DARRAY(self), i);
    darray_remove(DARRAY(self), i);
    RETURN(v != NULL ? LK_OBJ(lk_number_new(VM, *v)) : NIL);
}
static void setB_vec_number_number(lk_object_t *self, lk_scope_t *local) {
    /*
    darray_set(DARRAY(self), CSIZE(ARG(0)), &CSIZE(ARG(1)));
    */
    RETURN(self);
}
void lk_vector_libinit(lk_vm_t *vm) {
    lk_object_t *vec = vm->t_vector, *number = vm->t_number;
    lk_lib_setGlobal("Vector", vec);
    lk_object_setcfunc_lk(vec, "at", at_vec_number, number, NULL);
    lk_object_setcfunc_lk(vec, "grade", grade_vec, NULL);
    lk_object_setcfunc_lk(vec, "insert!", insertB_vec_number_number, number, number, NULL);
    lk_object_setcfunc_lk(vec, "remove!", removeB_vec_number, number, NULL);
    lk_object_setcfunc_lk(vec, "set!", setB_vec_number_number, number, number, NULL);
}
