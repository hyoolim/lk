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
    DBL(self) = DBL(parent);
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
LK_LIBRARY_DEFINECFUNCTION(add__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) + INT(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(addB__fi_fi) {
    INT(self) += INT(ARG(0));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(divide__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) / INT(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(divideB__fi_fi) {
    INT(self) /= INT(ARG(0));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(eq__fi_fi) {
    RETURN(INT(self) == INT(ARG(0)) ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(gt__fi_fi) {
    RETURN(INT(self) > INT(ARG(0)) ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(lt__fi_fi) {
    RETURN(INT(self) < INT(ARG(0)) ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(mod__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) % INT(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(modB__fi_fi) {
    INT(self) %= INT(ARG(0));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(multiply__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) * INT(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(multiplyB__fi_fi) {
    INT(self) *= INT(ARG(0));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(negate__fi) {
    RETURN(lk_fi_new(VM, -INT(self)));
}
LK_LIBRARY_DEFINECFUNCTION(negative__fi) {
    RETURN(INT(self) < 0 ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(positive__fi) {
    RETURN(INT(self) > 0 ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(subtract__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) - INT(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(subtractB__fi_fi) {
    INT(self) -= INT(ARG(0));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(to_real__fi) {
    RETURN(lk_fr_new(VM, (double)INT(self)));
}
LK_LIBRARY_DEFINECFUNCTION(to_string__fi_str) {
    const char *fmt = array_tocstr(LIST(ARG(0)));
    char *ret;
    lk_string_t *kcret;
    asprintf(&ret, fmt, INT(self));
    kcret = lk_string_newfromcstr(VM, ret);
    free(ret);
    RETURN(kcret);
}
LK_LIBRARY_DEFINECFUNCTION(zero__fi) {
    RETURN(INT(self) == 0 ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(add__fr_fr) {
    RETURN(lk_fr_new(VM, DBL(self) + DBL(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(addB__fr_fr) {
    DBL(self) += DBL(ARG(0));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(divide__fr_fr) {
    RETURN(lk_fr_new(VM, DBL(self) / DBL(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(divideB__fr_fr) {
    DBL(self) /= DBL(ARG(0));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(eq__fr_fr) {
    RETURN(DBL(self) == DBL(ARG(0)) ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(gt__fr_fr) {
    RETURN(DBL(self) > DBL(ARG(0)) ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(lt__fr_fr) {
    RETURN(DBL(self) < DBL(ARG(0)) ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(mod__fr_fr) {
    double x = DBL(self), y = DBL(ARG(0));
    RETURN(lk_fr_new(VM, x - y * (long)(x / y)));
}
LK_LIBRARY_DEFINECFUNCTION(modB__fr_fr) {
    double x = DBL(self), y = DBL(ARG(0));
    DBL(self) = x - y * (long)(x / y);
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(multiply__fr_fr) {
    RETURN(lk_fr_new(VM, DBL(self) * DBL(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(multiplyB__fr_fr) {
    DBL(self) *= DBL(ARG(0));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(negate__fr) {
    RETURN(lk_fr_new(VM, -DBL(self)));
}
LK_LIBRARY_DEFINECFUNCTION(negative__fr) {
    RETURN(DBL(self) < 0.0 ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(positive__fr) {
    RETURN(DBL(self) > 0.0 ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(subtract__fr_fr) {
    RETURN(lk_fr_new(VM, DBL(self) - DBL(ARG(0))));
}
LK_LIBRARY_DEFINECFUNCTION(subtractB__fr_fr) {
    DBL(self) -= DBL(ARG(0));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(to_integer__fr) {
    RETURN(lk_fi_new(VM, (int)DBL(self)));
}
LK_LIBRARY_DEFINECFUNCTION(to_string__fr_str) {
    const char *fmt = array_tocstr(LIST(ARG(0)));
    char *ret;
    lk_string_t *kcret;
    asprintf(&ret, fmt, DBL(self));
    kcret = lk_string_newfromcstr(VM, ret);
    free(ret);
    RETURN(kcret);
}
LK_LIBRARY_DEFINECFUNCTION(zero__fr) {
    RETURN(DBL(self) == 0.0 ? T : F);
}
LK_EXT_DEFINIT(lk_fixnum_extinitfuncs) {
    lk_object_t *num = vm->t_number, *str = vm->t_string,
                *i = vm->t_int, *fi = vm->t_fi,
                *f = vm->t_real, *fr = vm->t_fr;
    /* */
    lk_library_setGlobal("Number", num);
    lk_library_setGlobal("PositiveInfinity", vm->t_pi);
    lk_library_setGlobal("NegativeInfinity", vm->t_ni);
    lk_library_setGlobal("Integer", i);
    lk_library_setGlobal("FixedInteger", fi);
    lk_library_setCFunction(fi, "+", add__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "+=", addB__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "<=>", subtract__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "%", divide__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "%=", divideB__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "==", eq__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, ">", gt__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "<", lt__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "mod", mod__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "mod!", modB__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "*", multiply__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "*=", multiplyB__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "negate", negate__fi, NULL);
    lk_library_setCFunction(fi, "negative?", negative__fi, NULL);
    lk_library_setCFunction(fi, "positive?", positive__fi, NULL);
    lk_library_setCFunction(fi, "-", subtract__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "-=", subtractB__fi_fi, fi, NULL);
    lk_library_setCFunction(fi, "toReal", to_real__fi, NULL);
    lk_library_setCFunction(fi, "toString", to_string__fi_str, str, NULL);
    lk_library_setCFunction(fi, "zero?", zero__fi, NULL);
    /* */
    lk_library_setGlobal("Real", f);
    lk_library_setGlobal("FixedReal", fr);
    lk_library_setCFunction(fr, "+", add__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "+=", addB__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "<=>", subtract__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "%", divide__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "%=", divideB__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "==", eq__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, ">", gt__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "<", lt__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "mod", mod__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "mod!", modB__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "*", multiply__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "*=", multiplyB__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "negate", negate__fr, NULL);
    lk_library_setCFunction(fr, "negative?", negative__fr, NULL);
    lk_library_setCFunction(fr, "positive?", positive__fr, NULL);
    lk_library_setCFunction(fr, "-", subtract__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "-=", subtractB__fr_fr, fr, NULL);
    lk_library_setCFunction(fr, "toInteger", to_integer__fr, NULL);
    lk_library_setCFunction(fr, "toString", to_string__fr_str, str, NULL);
    lk_library_setCFunction(fr, "zero?", zero__fr, NULL);
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
