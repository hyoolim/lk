#include "vec.h"
#include "ext.h"
#include "num.h"

/* ext map - types */
void lk_vec_typeinit(lk_vm_t *vm) {
    vm->t_vec = lk_obj_alloc(vm->t_seq);
    darray_fin(DARRAY(vm->t_vec));
    darray_init(DARRAY(vm->t_vec), sizeof(int), 16);
}

/* ext map - funcs */
static void at_vec_num(lk_obj_t *self, lk_scope_t *local) {
    int *v = darray_get(DARRAY(self), CSIZE(ARG(0)));
    RETURN(v != NULL ? LK_OBJ(lk_num_new(VM, *v)) : NIL);
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
static void grade_vec(lk_obj_t *self, lk_scope_t *local) {
    lk_vec_t *indexes = LK_VECTOR(lk_obj_alloc(VM->t_vec));
    darray_t *sl = DARRAY(self), *il = DARRAY(indexes);
    darray_resize(il, LIST_COUNT(sl));
    LIST_EACH(il, i, v, *(int *)v = i);
    quicksort_hoare(sl, il, 0, LIST_COUNT(il) - 1);
    RETURN(indexes);
}
static void insertB_vec_num_num(lk_obj_t *self, lk_scope_t *local) {
    /*
    darray_insert(DARRAY(self), CSIZE(ARG(0)), &CSIZE(ARG(1)));
    */
    RETURN(self);
}
static void removeB_vec_num(lk_obj_t *self, lk_scope_t *local) {
    int i = CSIZE(ARG(0));
    int *v = darray_get(DARRAY(self), i);
    darray_remove(DARRAY(self), i);
    RETURN(v != NULL ? LK_OBJ(lk_num_new(VM, *v)) : NIL);
}
static void setB_vec_num_num(lk_obj_t *self, lk_scope_t *local) {
    /*
    darray_set(DARRAY(self), CSIZE(ARG(0)), &CSIZE(ARG(1)));
    */
    RETURN(self);
}
void lk_vec_libinit(lk_vm_t *vm) {
    lk_obj_t *vec = vm->t_vec, *num = vm->t_num;
    lk_lib_setGlobal("Vector", vec);
    lk_obj_set_cfunc_lk(vec, "at", at_vec_num, num, NULL);
    lk_obj_set_cfunc_lk(vec, "grade", grade_vec, NULL);
    lk_obj_set_cfunc_lk(vec, "insert!", insertB_vec_num_num, num, num, NULL);
    lk_obj_set_cfunc_lk(vec, "remove!", removeB_vec_num, num, NULL);
    lk_obj_set_cfunc_lk(vec, "set!", setB_vec_num_num, num, num, NULL);
}
