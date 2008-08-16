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
    vm->t_number = lk_obj_alloc(vm->t_obj);
    vm->t_pi = lk_obj_alloc(vm->t_obj);
    vm->t_ni = lk_obj_alloc(vm->t_obj);
    vm->t_int = lk_obj_alloc(vm->t_number);
    vm->t_real = lk_obj_alloc(vm->t_number);
    vm->t_fi = lk_obj_allocwithsize(vm->t_int, sizeof(lk_fi_t));
    lk_obj_setallocfunc(vm->t_fi, alloc__fi);
    vm->t_fr = lk_obj_allocwithsize(vm->t_real, sizeof(lk_fr_t));
    lk_obj_setallocfunc(vm->t_fr, alloc__fr);
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
static LK_EXT_DEFCFUNC(mod__fi_fi) {
    RETURN(lk_fi_new(VM, INT(self) % INT(ARG(0))));
}
static LK_EXT_DEFCFUNC(modB__fi_fi) {
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
static LK_EXT_DEFCFUNC(mod__fr_fr) {
    double x = DBL(self), y = DBL(ARG(0));
    RETURN(lk_fr_new(VM, x - y * (long)(x / y)));
}
static LK_EXT_DEFCFUNC(modB__fr_fr) {
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
    lk_obj_t *num = vm->t_number, *str = vm->t_string,
                *i = vm->t_int, *fi = vm->t_fi,
                *f = vm->t_real, *fr = vm->t_fr;
    /* */
    lk_ext_global("Number", num);
    lk_ext_global("PositiveInfinity", vm->t_pi);
    lk_ext_global("NegativeInfinity", vm->t_ni);
    lk_ext_global("Integer", i);
    lk_ext_global("FixedInteger", fi);
    lk_ext_cfunc(fi, "+", add__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "+=", addB__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "<=>", subtract__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "%", divide__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "%=", divideB__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "==", eq__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, ">", gt__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "<", lt__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "mod", mod__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "mod!", modB__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "*", multiply__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "*=", multiplyB__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "negate", negate__fi, NULL);
    lk_ext_cfunc(fi, "negative?", negative__fi, NULL);
    lk_ext_cfunc(fi, "positive?", positive__fi, NULL);
    lk_ext_cfunc(fi, "-", subtract__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "-=", subtractB__fi_fi, fi, NULL);
    lk_ext_cfunc(fi, "toReal", to_real__fi, NULL);
    lk_ext_cfunc(fi, "toString", to_string__fi_str, str, NULL);
    lk_ext_cfunc(fi, "zero?", zero__fi, NULL);
    /* */
    lk_ext_global("Real", f);
    lk_ext_global("FixedReal", fr);
    lk_ext_cfunc(fr, "+", add__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "+=", addB__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "<=>", subtract__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "%", divide__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "%=", divideB__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "==", eq__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, ">", gt__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "<", lt__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "mod", mod__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "mod!", modB__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "*", multiply__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "*=", multiplyB__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "negate", negate__fr, NULL);
    lk_ext_cfunc(fr, "negative?", negative__fr, NULL);
    lk_ext_cfunc(fr, "positive?", positive__fr, NULL);
    lk_ext_cfunc(fr, "-", subtract__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "-=", subtractB__fr_fr, fr, NULL);
    lk_ext_cfunc(fr, "toInteger", to_integer__fr, NULL);
    lk_ext_cfunc(fr, "toString", to_string__fr_str, str, NULL);
    lk_ext_cfunc(fr, "zero?", zero__fr, NULL);
}

/* new */
lk_fi_t *lk_fi_new(lk_vm_t *vm, int i) {
    lk_fi_t *self = LK_FI(lk_obj_alloc(vm->t_fi));
    self->i = i;
    return self;
}
lk_fr_t *lk_fr_new(lk_vm_t *vm, double r) {
    lk_fr_t *self = LK_FR(lk_obj_alloc(vm->t_fr));
    self->r = r;
    return self;
}
