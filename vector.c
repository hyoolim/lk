#include "vector.h"
#include "ext.h"
#include "number.h"

/* ext map - types */
LK_LIB_DEFINEINIT(lk_vector_libPreInit) {
    vm->t_vector = lk_object_alloc(vm->t_seq);
    darray_fin(DARRAY(vm->t_vector));
    darray_init(DARRAY(vm->t_vector), sizeof(int), 16);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(at_vec_number) {
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
LK_LIB_DEFINECFUNC(grade_vec) {
    lk_vector_t *indexes = LK_VECTOR(lk_object_alloc(VM->t_vector));
    darray_t *sl = DARRAY(self), *il = DARRAY(indexes);
    darray_resize(il, LIST_COUNT(sl));
    LIST_EACH(il, i, v, *(int *)v = i);
    quicksort_hoare(sl, il, 0, LIST_COUNT(il) - 1);
    RETURN(indexes);
}
LK_LIB_DEFINECFUNC(insertB_vec_number_number) {
    /*
    darray_insert(DARRAY(self), CSIZE(ARG(0)), &CSIZE(ARG(1)));
    */
    RETURN(self);
}
LK_LIB_DEFINECFUNC(removeB_vec_number) {
    int i = CSIZE(ARG(0));
    int *v = darray_get(DARRAY(self), i);
    darray_remove(DARRAY(self), i);
    RETURN(v != NULL ? LK_OBJ(lk_number_new(VM, *v)) : NIL);
}
LK_LIB_DEFINECFUNC(setB_vec_number_number) {
    /*
    darray_set(DARRAY(self), CSIZE(ARG(0)), &CSIZE(ARG(1)));
    */
    RETURN(self);
}
LK_LIB_DEFINEINIT(lk_vector_libInit) {
    lk_object_t *vec = vm->t_vector, *number = vm->t_number;
    lk_lib_setGlobal("Vector", vec);
    lk_lib_setCFunc(vec, "at", at_vec_number, number, NULL);
    lk_lib_setCFunc(vec, "grade", grade_vec, NULL);
    lk_lib_setCFunc(vec, "insert!", insertB_vec_number_number, number, number, NULL);
    lk_lib_setCFunc(vec, "remove!", removeB_vec_number, number, NULL);
    lk_lib_setCFunc(vec, "set!", setB_vec_number_number, number, number, NULL);
}
