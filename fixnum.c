#include "fixnum.h"
#include "ext.h"
#include "string.h"

/* non-ansi */
int snprintf(char *str, size_t size, const char *format, ...);

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__fi) {
    INT(self) = INT(parent);
}
static LK_OBJ_DEFALLOCFUNC(alloc__fr) {
    DOUBLE(self) = DOUBLE(parent);
}
LK_EXT_DEFINIT(lk_fixnum_extinittypes) {
    vm->t_number = lk_object_alloc(vm->t_obj);
    vm->t_pi = lk_object_alloc(vm->t_obj);
    vm->t_ni = lk_object_alloc(vm->t_obj);
    vm->t_int = lk_object_alloc(vm->t_number);
    vm->t_real = lk_object_alloc(vm->t_number);
    vm->t_fi = lk_object_allocwithsize(vm->t_int, sizeof(lk_fi_t));
    lk_object_setallocfunc(vm->t_fi, alloc__fi);
    vm->t_fr = lk_object_allocwithsize(vm->t_real, sizeof(lk_fr_t));
    lk_object_setallocfunc(vm->t_fr, alloc__fr);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(add__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) + INT(ARG(0))));
}
LK_LIB_DEFINECFUNC(addB__fi_fi) {
    INT(self) += INT(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(divide__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) / INT(ARG(0))));
}
LK_LIB_DEFINECFUNC(divideB__fi_fi) {
    INT(self) /= INT(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(eq__fi_fi) {
    RETURN(INT(self) == INT(ARG(0)) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(gt__fi_fi) {
    RETURN(INT(self) > INT(ARG(0)) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(lt__fi_fi) {
    RETURN(INT(self) < INT(ARG(0)) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(mod__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) % INT(ARG(0))));
}
LK_LIB_DEFINECFUNC(modB__fi_fi) {
    INT(self) %= INT(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(multiply__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) * INT(ARG(0))));
}
LK_LIB_DEFINECFUNC(multiplyB__fi_fi) {
    INT(self) *= INT(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(negate__fi) {
    RETURN(lk_fi_new(VM, -INT(self)));
}
LK_LIB_DEFINECFUNC(negative__fi) {
    RETURN(INT(self) < 0 ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(positive__fi) {
    RETURN(INT(self) > 0 ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(subtract__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) - INT(ARG(0))));
}
LK_LIB_DEFINECFUNC(subtractB__fi_fi) {
    INT(self) -= INT(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(to_real__fi) {
    RETURN(lk_fr_new(VM, (double)INT(self)));
}
LK_LIB_DEFINECFUNC(to_string__fi_str) {
    const char *fmt = darray_toCString(DARRAY(ARG(0)));
    char *ret;
    lk_string_t *kcret;
    asprintf(&ret, fmt, INT(self));
    kcret = lk_string_newFromCString(VM, ret);
    free(ret);
    RETURN(kcret);
}
LK_LIB_DEFINECFUNC(zero__fi) {
    RETURN(INT(self) == 0 ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(add__fr_fr) {
    RETURN(lk_fr_new(VM, DOUBLE(self) + DOUBLE(ARG(0))));
}
LK_LIB_DEFINECFUNC(addB__fr_fr) {
    DOUBLE(self) += DOUBLE(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(divide__fr_fr) {
    RETURN(lk_fr_new(VM, DOUBLE(self) / DOUBLE(ARG(0))));
}
LK_LIB_DEFINECFUNC(divideB__fr_fr) {
    DOUBLE(self) /= DOUBLE(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(eq__fr_fr) {
    RETURN(DOUBLE(self) == DOUBLE(ARG(0)) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(gt__fr_fr) {
    RETURN(DOUBLE(self) > DOUBLE(ARG(0)) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(lt__fr_fr) {
    RETURN(DOUBLE(self) < DOUBLE(ARG(0)) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(mod__fr_fr) {
    double x = DOUBLE(self), y = DOUBLE(ARG(0));
    RETURN(lk_fr_new(VM, x - y * (long)(x / y)));
}
LK_LIB_DEFINECFUNC(modB__fr_fr) {
    double x = DOUBLE(self), y = DOUBLE(ARG(0));
    DOUBLE(self) = x - y * (long)(x / y);
    RETURN(self);
}
LK_LIB_DEFINECFUNC(multiply__fr_fr) {
    RETURN(lk_fr_new(VM, DOUBLE(self) * DOUBLE(ARG(0))));
}
LK_LIB_DEFINECFUNC(multiplyB__fr_fr) {
    DOUBLE(self) *= DOUBLE(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(negate__fr) {
    RETURN(lk_fr_new(VM, -DOUBLE(self)));
}
LK_LIB_DEFINECFUNC(negative__fr) {
    RETURN(DOUBLE(self) < 0.0 ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(positive__fr) {
    RETURN(DOUBLE(self) > 0.0 ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(subtract__fr_fr) {
    RETURN(lk_fr_new(VM, DOUBLE(self) - DOUBLE(ARG(0))));
}
LK_LIB_DEFINECFUNC(subtractB__fr_fr) {
    DOUBLE(self) -= DOUBLE(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(to_integer__fr) {
    RETURN(lk_fi_new(VM, (int)DOUBLE(self)));
}
LK_LIB_DEFINECFUNC(to_string__fr_str) {
    const char *fmt = darray_toCString(DARRAY(ARG(0)));
    char *ret;
    lk_string_t *kcret;
    asprintf(&ret, fmt, DOUBLE(self));
    kcret = lk_string_newFromCString(VM, ret);
    free(ret);
    RETURN(kcret);
}
LK_LIB_DEFINECFUNC(zero__fr) {
    RETURN(DOUBLE(self) == 0.0 ? TRUE : FALSE);
}
LK_EXT_DEFINIT(lk_fixnum_extinitfuncs) {
    lk_object_t *num = vm->t_number, *str = vm->t_string,
                *i = vm->t_int, *fi = vm->t_fi,
                *f = vm->t_real, *fr = vm->t_fr;
    /* */
    lk_lib_setGlobal("Number", num);
    lk_lib_setGlobal("PositiveInfinity", vm->t_pi);
    lk_lib_setGlobal("NegativeInfinity", vm->t_ni);
    lk_lib_setGlobal("Integer", i);
    lk_lib_setGlobal("FixedInteger", fi);
    lk_lib_setCFunc(fi, "+", add__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "+=", addB__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "<=>", subtract__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "%", divide__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "%=", divideB__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "==", eq__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, ">", gt__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "<", lt__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "mod", mod__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "mod!", modB__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "*", multiply__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "*=", multiplyB__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "negate", negate__fi, NULL);
    lk_lib_setCFunc(fi, "negative?", negative__fi, NULL);
    lk_lib_setCFunc(fi, "positive?", positive__fi, NULL);
    lk_lib_setCFunc(fi, "-", subtract__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "-=", subtractB__fi_fi, fi, NULL);
    lk_lib_setCFunc(fi, "toReal", to_real__fi, NULL);
    lk_lib_setCFunc(fi, "toString", to_string__fi_str, str, NULL);
    lk_lib_setCFunc(fi, "zero?", zero__fi, NULL);
    /* */
    lk_lib_setGlobal("Real", f);
    lk_lib_setGlobal("FixedReal", fr);
    lk_lib_setCFunc(fr, "+", add__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "+=", addB__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "<=>", subtract__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "%", divide__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "%=", divideB__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "==", eq__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, ">", gt__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "<", lt__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "mod", mod__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "mod!", modB__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "*", multiply__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "*=", multiplyB__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "negate", negate__fr, NULL);
    lk_lib_setCFunc(fr, "negative?", negative__fr, NULL);
    lk_lib_setCFunc(fr, "positive?", positive__fr, NULL);
    lk_lib_setCFunc(fr, "-", subtract__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "-=", subtractB__fr_fr, fr, NULL);
    lk_lib_setCFunc(fr, "toInteger", to_integer__fr, NULL);
    lk_lib_setCFunc(fr, "toString", to_string__fr_str, str, NULL);
    lk_lib_setCFunc(fr, "zero?", zero__fr, NULL);
}

/* new */
lk_fi_t *lk_fi_new(lk_vm_t *vm, int i) {
    lk_fi_t *self = LK_FI(lk_object_alloc(vm->t_fi));
    self->i = i;
    return self;
}
lk_fr_t *lk_fr_new(lk_vm_t *vm, double r) {
    lk_fr_t *self = LK_FR(lk_object_alloc(vm->t_fr));
    self->r = r;
    return self;
}
