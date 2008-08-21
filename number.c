#include "number.h"
#include "ext.h"
#include "string.h"
#include <math.h>

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc__number) {
    CNUMBER(self) = CNUMBER(parent);
}
LK_LIB_DEFINEINIT(lk_number_libPreInit) {
    vm->t_number = lk_object_allocwithsize(vm->t_obj, sizeof(lk_number_t));
    lk_object_setallocfunc(vm->t_number, alloc__number);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(abs__number) {
    RETURN(lk_number_new(VM, fabs(CNUMBER(self))));
}
LK_LIB_DEFINECFUNC(add__number_number) {
    RETURN(lk_number_new(VM, CNUMBER(self) + CNUMBER(ARG(0))));
}
LK_LIB_DEFINECFUNC(addB__number_number) {
    CNUMBER(self) += CNUMBER(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(ceil__number) {
    RETURN(lk_number_new(VM, ceil(CNUMBER(self))));
}
LK_LIB_DEFINECFUNC(divide__number_number) {
    RETURN(lk_number_new(VM, CNUMBER(self) / CNUMBER(ARG(0))));
}
LK_LIB_DEFINECFUNC(divideB__number_number) {
    CNUMBER(self) /= CNUMBER(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(eq__number_number) {
    RETURN(CNUMBER(self) == CNUMBER(ARG(0)) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(floor__number) {
    RETURN(lk_number_new(VM, floor(CNUMBER(self))));
}
LK_LIB_DEFINECFUNC(gt__number_number) {
    RETURN(CNUMBER(self) > CNUMBER(ARG(0)) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(integerQuestion__number) {
    double i;
    RETURN(modf(CNUMBER(self), &i) == 0.0 ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(lt__number_number) {
    RETURN(CNUMBER(self) < CNUMBER(ARG(0)) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(mod__number_number) {
    RETURN(lk_number_new(VM, fmod(CNUMBER(self), CNUMBER(ARG(0)))));
}
LK_LIB_DEFINECFUNC(modB__number_number) {
    CNUMBER(self) = fmod(CNUMBER(self), CNUMBER(ARG(0)));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(multiply__number_number) {
    RETURN(lk_number_new(VM, CNUMBER(self) * CNUMBER(ARG(0))));
}
LK_LIB_DEFINECFUNC(multiplyB__number_number) {
    CNUMBER(self) *= CNUMBER(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(negate__number) {
    RETURN(lk_number_new(VM, -CNUMBER(self)));
}
LK_LIB_DEFINECFUNC(round__number) {
    double f = CNUMBER(self), i = floor(f);
    RETURN(lk_number_new(VM, f - i > 0.5 ? i + 1 : f - i < -0.5 ? i - 1 : i));
}
LK_LIB_DEFINECFUNC(subtract__number_number) {
    RETURN(lk_number_new(VM, CNUMBER(self) - CNUMBER(ARG(0))));
}
LK_LIB_DEFINECFUNC(subtractB__number_number) {
    CNUMBER(self) -= CNUMBER(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(to_string__number_number_number) {
    int i = CNUMBER(ARG(0)), f = CNUMBER(ARG(1));
    char cString[200];
    snprintf(cString, 200, "%*.*f", i, f, CNUMBER(self));
    RETURN(lk_string_newFromCString(VM, cString));
}
LK_LIB_DEFINEINIT(lk_number_libInit) {
    lk_object_t *number = vm->t_number;
    lk_lib_setGlobal("Number", number);
    lk_lib_setCFunc(number, "abs", abs__number, NULL);
    lk_lib_setCFunc(number, "+", add__number_number, number, NULL);
    lk_lib_setCFunc(number, "+=", addB__number_number, number, NULL);
    lk_lib_setCFunc(number, "ceil", ceil__number, NULL);
    lk_lib_setCFunc(number, "<=>", subtract__number_number, number, NULL);
    lk_lib_setCFunc(number, "%", divide__number_number, number, NULL);
    lk_lib_setCFunc(number, "%=", divideB__number_number, number, NULL);
    lk_lib_setCFunc(number, "==", eq__number_number, number, NULL);
    lk_lib_setCFunc(number, "floor", floor__number, NULL);
    lk_lib_setCFunc(number, ">", gt__number_number, number, NULL);
    lk_lib_setCFunc(number, "integer?", integerQuestion__number, NULL);
    lk_lib_setCFunc(number, "<", lt__number_number, number, NULL);
    lk_lib_setCFunc(number, "mod", mod__number_number, number, NULL);
    lk_lib_setCFunc(number, "mod!", modB__number_number, number, NULL);
    lk_lib_setCFunc(number, "*", multiply__number_number, number, NULL);
    lk_lib_setCFunc(number, "*=", multiplyB__number_number, number, NULL);
    lk_lib_setCFunc(number, "negate", negate__number, NULL);
    lk_lib_setCFunc(number, "round", round__number, NULL);
    lk_lib_setCFunc(number, "-", subtract__number_number, number, NULL);
    lk_lib_setCFunc(number, "-=", subtractB__number_number, number, NULL);
    lk_lib_setCFunc(number, "toString", to_string__number_number_number, number, number, NULL);
}

/* new */
lk_number_t *lk_number_new(lk_vm_t *vm, double number) {
    lk_number_t *self = LK_NUMBER(lk_object_alloc(vm->t_number));
    CNUMBER(self) = number;
    return self;
}
