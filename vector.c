#include "vector.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_Vector_extinittypes) {
    vm->t_vector = lk_Object_alloc(vm->t_glist);
    Sequence_fin(LIST(vm->t_vector));
    Sequence_init(LIST(vm->t_vector), sizeof(int), 16);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(at__vec_fi) {
    int *v = Sequence_get(LIST(self), INT(ARG(0)));
    RETURN(v != NULL ? LK_OBJ(lk_Fi_new(VM, *v)) : N);
}
#define AT(i) (*(int *)LIST_AT(values, *(int *)LIST_AT(indexes, (i))))
#define SWAP(x, y) do { \
    t = *(int *)LIST_AT(indexes, (x)); \
    *(int *)LIST_AT(indexes, (x)) = *(int *)LIST_AT(indexes, (y)); \
    *(int *)LIST_AT(indexes, (y)) = t; \
} while(0)
static void quicksort_hoare(Sequence_t *values, Sequence_t *indexes, int low, int hi) {
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
LK_LIBRARY_DEFINECFUNCTION(grade__vec) {
    lk_Vector_t *indexes = LK_VECTOR(lk_Object_alloc(VM->t_vector));
    Sequence_t *sl = LIST(self), *il = LIST(indexes);
    Sequence_resize(il, LIST_COUNT(sl));
    LIST_EACH(il, i, v, *(int *)v = i);
    quicksort_hoare(sl, il, 0, LIST_COUNT(il) - 1);
    RETURN(indexes);
}
LK_LIBRARY_DEFINECFUNCTION(insertB__vec_fi_fi) {
    Sequence_insert(LIST(self), INT(ARG(0)), &INT(ARG(1)));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(removeB__vec_fi) {
    int i = INT(ARG(0));
    int *v = Sequence_get(LIST(self), i);
    Sequence_remove(LIST(self), i);
    RETURN(v != NULL ? LK_OBJ(lk_Fi_new(VM, *v)) : N);
}
LK_LIBRARY_DEFINECFUNCTION(setB__vec_fi_fi) {
    Sequence_set(LIST(self), INT(ARG(0)), &INT(ARG(1)));
    RETURN(self);
}
LK_EXT_DEFINIT(lk_Vector_extinitfuncs) {
    lk_Object_t *vec = vm->t_vector, *fi = vm->t_fi;
    lk_Library_setGlobal("Vector", vec);
    lk_Library_setCFunction(vec, "at", at__vec_fi, fi, NULL);
    lk_Library_setCFunction(vec, "grade", grade__vec, NULL);
    lk_Library_setCFunction(vec, "insert!", insertB__vec_fi_fi, fi, fi, NULL);
    lk_Library_setCFunction(vec, "remove!", removeB__vec_fi, fi, NULL);
    lk_Library_setCFunction(vec, "set!", setB__vec_fi_fi, fi, fi, NULL);
}
