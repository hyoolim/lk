#include "vector.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_vector_extinittypes) {
    vm->t_vector = lk_obj_alloc(vm->t_glist);
    list_fin(LIST(vm->t_vector));
    list_init(LIST(vm->t_vector), sizeof(int), 16);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(at__vec_fi) {
    int *v = list_get(LIST(self), INT(ARG(0)));
    RETURN(v != NULL ? LK_OBJ(lk_fi_new(VM, *v)) : N);
}
#define AT(i) (*(int *)LIST_AT(values, *(int *)LIST_AT(indexes, (i))))
#define SWAP(x, y) do { \
    t = *(int *)LIST_AT(indexes, (x)); \
    *(int *)LIST_AT(indexes, (x)) = *(int *)LIST_AT(indexes, (y)); \
    *(int *)LIST_AT(indexes, (y)) = t; \
} while(0)
static void quicksort_hoare(list_t *values, list_t *indexes, int low, int hi) {
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
static LK_EXT_DEFCFUNC(grade__vec) {
    lk_vector_t *indexes = LK_VECTOR(lk_obj_alloc(VM->t_vector));
    list_t *sl = LIST(self), *il = LIST(indexes);
    list_resize(il, LIST_COUNT(sl));
    LIST_EACH(il, i, v, *(int *)v = i);
    quicksort_hoare(sl, il, 0, LIST_COUNT(il) - 1);
    RETURN(indexes);
}
static LK_EXT_DEFCFUNC(insertB__vec_fi_fi) {
    list_insert(LIST(self), INT(ARG(0)), &INT(ARG(1)));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(removeB__vec_fi) {
    int i = INT(ARG(0));
    int *v = list_get(LIST(self), i);
    list_remove(LIST(self), i);
    RETURN(v != NULL ? LK_OBJ(lk_fi_new(VM, *v)) : N);
}
static LK_EXT_DEFCFUNC(setB__vec_fi_fi) {
    list_set(LIST(self), INT(ARG(0)), &INT(ARG(1)));
    RETURN(self);
}
LK_EXT_DEFINIT(lk_vector_extinitfuncs) {
    lk_obj_t *vec = vm->t_vector, *fi = vm->t_fi;
    lk_ext_global("Vector", vec);
    lk_ext_cfunc(vec, "at", at__vec_fi, fi, NULL);
    lk_ext_cfunc(vec, "grade", grade__vec, NULL);
    lk_ext_cfunc(vec, "insert!", insertB__vec_fi_fi, fi, fi, NULL);
    lk_ext_cfunc(vec, "remove!", removeB__vec_fi, fi, NULL);
    lk_ext_cfunc(vec, "set!", setB__vec_fi_fi, fi, fi, NULL);
}
