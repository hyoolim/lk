#include "fixnum.h"
#include "ext.h"
#include "string.h"

/* non-ansi */
int snprintf(char *str, size_t size, const char *format, ...);

/* ext map - types */
static LK_OBJECT_DEFALLOCFUNC(alloc__fi) {
    INT(self) = INT(proto);
}
static LK_OBJECT_DEFALLOCFUNC(alloc__fr) {
    DBL(self) = DBL(proto);
}
LK_EXT_DEFINIT(lk_fixnum_extinittypes) {
    vm->t_number = lk_object_alloc(vm->t_object);
    vm->t_pi = lk_object_alloc(vm->t_object);
    vm->t_ni = lk_object_alloc(vm->t_object);
    vm->t_int = lk_object_alloc(vm->t_number);
    vm->t_real = lk_object_alloc(vm->t_number);
    vm->t_fi = lk_object_allocwithsize(vm->t_int, sizeof(lk_fi_t));
    lk_object_setallocfunc(vm->t_fi, alloc__fi);
    vm->t_fr = lk_object_allocwithsize(vm->t_real, sizeof(lk_fr_t));
    lk_object_setallocfunc(vm->t_fr, alloc__fr);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(add__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) + INT(ARG(0))));
}
static LK_EXT_DEFCFUNC(addB__fi_fi) {
    INT(self) += INT(ARG(0));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(divide__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) / INT(ARG(0))));
}
static LK_EXT_DEFCFUNC(divideB__fi_fi) {
    INT(self) /= INT(ARG(0));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(eq__fi_fi) {
    RETURN(INT(self) == INT(ARG(0)) ? T : F);
}
static LK_EXT_DEFCFUNC(gt__fi_fi) {
    RETURN(INT(self) > INT(ARG(0)) ? T : F);
}
static LK_EXT_DEFCFUNC(lt__fi_fi) {
    RETURN(INT(self) < INT(ARG(0)) ? T : F);
}
static LK_EXT_DEFCFUNC(modulo__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) % INT(ARG(0))));
}
static LK_EXT_DEFCFUNC(moduloB__fi_fi) {
    INT(self) %= INT(ARG(0));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(multiply__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) * INT(ARG(0))));
}
static LK_EXT_DEFCFUNC(multiplyB__fi_fi) {
    INT(self) *= INT(ARG(0));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(negate__fi) {
    RETURN(lk_fi_new(VM, -INT(self)));
}
static LK_EXT_DEFCFUNC(negative__fi) {
    RETURN(INT(self) < 0 ? T : F);
}
static LK_EXT_DEFCFUNC(positive__fi) {
    RETURN(INT(self) > 0 ? T : F);
}
static LK_EXT_DEFCFUNC(subtract__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) - INT(ARG(0))));
}
static LK_EXT_DEFCFUNC(subtractB__fi_fi) {
    INT(self) -= INT(ARG(0));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(to_real__fi) {
    RETURN(lk_fr_new(VM, (double)INT(self)));
}
static LK_EXT_DEFCFUNC(to_string__fi_str) {
    const char *fmt = list_tocstr(LIST(ARG(0)));
    char *ret;
    lk_string_t *kcret;
    asprintf(&ret, fmt, INT(self));
    kcret = lk_string_newfromcstr(VM, ret);
    free(ret);
    RETURN(kcret);
}
static LK_EXT_DEFCFUNC(zero__fi) {
    RETURN(INT(self) == 0 ? T : F);
}
static LK_EXT_DEFCFUNC(add__fr_fr) {
    RETURN(lk_fr_new(VM, DBL(self) + DBL(ARG(0))));
}
static LK_EXT_DEFCFUNC(addB__fr_fr) {
    DBL(self) += DBL(ARG(0));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(divide__fr_fr) {
    RETURN(lk_fr_new(VM, DBL(self) / DBL(ARG(0))));
}
static LK_EXT_DEFCFUNC(divideB__fr_fr) {
    DBL(self) /= DBL(ARG(0));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(eq__fr_fr) {
    RETURN(DBL(self) == DBL(ARG(0)) ? T : F);
}
static LK_EXT_DEFCFUNC(gt__fr_fr) {
    RETURN(DBL(self) > DBL(ARG(0)) ? T : F);
}
static LK_EXT_DEFCFUNC(lt__fr_fr) {
    RETURN(DBL(self) < DBL(ARG(0)) ? T : F);
}
static LK_EXT_DEFCFUNC(modulo__fr_fr) {
    double x = DBL(self), y = DBL(ARG(0));
    RETURN(lk_fr_new(VM, x - y * (long)(x / y)));
}
static LK_EXT_DEFCFUNC(moduloB__fr_fr) {
    double x = DBL(self), y = DBL(ARG(0));
    DBL(self) = x - y * (long)(x / y);
    RETURN(self);
}
static LK_EXT_DEFCFUNC(multiply__fr_fr) {
    RETURN(lk_fr_new(VM, DBL(self) * DBL(ARG(0))));
}
static LK_EXT_DEFCFUNC(multiplyB__fr_fr) {
    DBL(self) *= DBL(ARG(0));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(negate__fr) {
    RETURN(lk_fr_new(VM, -DBL(self)));
}
static LK_EXT_DEFCFUNC(negative__fr) {
    RETURN(DBL(self) < 0.0 ? T : F);
}
static LK_EXT_DEFCFUNC(positive__fr) {
    RETURN(DBL(self) > 0.0 ? T : F);
}
static LK_EXT_DEFCFUNC(subtract__fr_fr) {
    RETURN(lk_fr_new(VM, DBL(self) - DBL(ARG(0))));
}
static LK_EXT_DEFCFUNC(subtractB__fr_fr) {
    DBL(self) -= DBL(ARG(0));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(to_integer__fr) {
    RETURN(lk_fi_new(VM, (int)DBL(self)));
}
static LK_EXT_DEFCFUNC(to_string__fr_str) {
    const char *fmt = list_tocstr(LIST(ARG(0)));
    char *ret;
    lk_string_t *kcret;
    asprintf(&ret, fmt, DBL(self));
    kcret = lk_string_newfromcstr(VM, ret);
    free(ret);
    RETURN(kcret);
}
static LK_EXT_DEFCFUNC(zero__fr) {
    RETURN(DBL(self) == 0.0 ? T : F);
}
LK_EXT_DEFINIT(lk_fixnum_extinitfuncs) {
    lk_object_t *num = vm->t_number, *str = vm->t_string,
                *i = vm->t_int, *fi = vm->t_fi,
                *f = vm->t_real, *fr = vm->t_fr;
    /* */
    lk_ext_global("Number", num);
    lk_ext_global("POSITIVE INFINITY", vm->t_pi);
    lk_ext_global("INF", vm->t_pi);
    lk_ext_global("NEGATIVE INFINITY", vm->t_ni);
    lk_ext_global("Integer", i);
    lk_ext_global("FixedInteger", fi);
    lk_ext_cfunc(fi, "add", add__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "add=", addB__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "cmp", subtract__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "divide", divide__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "divide=", divideB__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "eq?", eq__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "gt?", gt__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "lt?", lt__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "modulo", modulo__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "modulo!", moduloB__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "multiply", multiply__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "multiply=", multiplyB__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "negate", negate__fi, NULL);
    lk_ext_cfunc(fi, "negative?", negative__fi, NULL);
    lk_ext_cfunc(fi, "positive?", positive__fi, NULL);
    lk_ext_cfunc(fi, "subtract", subtract__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "subtract=", subtractB__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "to real", to_real__fi, NULL);
    lk_ext_cfunc(fi, "to string", to_string__fi_str, str, NULL);
    lk_ext_cfunc(fi, "zero?", zero__fi, NULL);
    /* */
    lk_ext_global("Real", f);
    lk_ext_global("FixedReal", fr);
    lk_ext_cfunc(fr, "add", add__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "add=", addB__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "cmp", subtract__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "divide", divide__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "divide=", divideB__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "eq?", eq__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "gt?", gt__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "lt?", lt__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "modulo", modulo__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "modulo!", moduloB__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "multiply", multiply__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "multiply=", multiplyB__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "negate", negate__fr, NULL);
    lk_ext_cfunc(fr, "negative?", negative__fr, NULL);
    lk_ext_cfunc(fr, "positive?", positive__fr, NULL);
    lk_ext_cfunc(fr, "subtract", subtract__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "subtract=", subtractB__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "to integer", to_integer__fr, NULL);
    lk_ext_cfunc(fr, "to string", to_string__fr_str, str, NULL);
    lk_ext_cfunc(fr, "zero?", zero__fr, NULL);
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
