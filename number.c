#include "number.h"
#include "ext.h"
#include "string.h"
#include <math.h>

/* ext map - types */
static LK_OBJ_DEFALLOCFUNC(alloc_number) {
    CNUMBER(self) = CNUMBER(parent);
}
LK_LIB_DEFINEINIT(lk_number_libPreInit) {
    vm->t_number = lk_object_allocWithSize(vm->t_object, sizeof(lk_number_t));
    lk_object_setallocfunc(vm->t_number, alloc_number);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(abs_number) {
    RETURN(lk_number_new(VM, fabs(CNUMBER(self))));
}
LK_LIB_DEFINECFUNC(add_number_number) {
    RETURN(lk_number_new(VM, CNUMBER(self) + CNUMBER(ARG(0))));
}
LK_LIB_DEFINECFUNC(addB_number_number) {
    CNUMBER(self) += CNUMBER(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(ceil_number) {
    RETURN(lk_number_new(VM, ceil(CNUMBER(self))));
}
LK_LIB_DEFINECFUNC(divide_number_number) {
    RETURN(lk_number_new(VM, CNUMBER(self) / CNUMBER(ARG(0))));
}
LK_LIB_DEFINECFUNC(divideB_number_number) {
    CNUMBER(self) /= CNUMBER(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(eq_number_number) {
    RETURN(CNUMBER(self) == CNUMBER(ARG(0)) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(floor_number) {
    RETURN(lk_number_new(VM, floor(CNUMBER(self))));
}
LK_LIB_DEFINECFUNC(gt_number_number) {
    RETURN(CNUMBER(self) > CNUMBER(ARG(0)) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(integerQuestion_number) {
    double i;
    RETURN(modf(CNUMBER(self), &i) == 0.0 ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(lt_number_number) {
    RETURN(CNUMBER(self) < CNUMBER(ARG(0)) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(mod_number_number) {
    RETURN(lk_number_new(VM, fmod(CNUMBER(self), CNUMBER(ARG(0)))));
}
LK_LIB_DEFINECFUNC(modB_number_number) {
    CNUMBER(self) = fmod(CNUMBER(self), CNUMBER(ARG(0)));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(multiply_number_number) {
    RETURN(lk_number_new(VM, CNUMBER(self) * CNUMBER(ARG(0))));
}
LK_LIB_DEFINECFUNC(multiplyB_number_number) {
    CNUMBER(self) *= CNUMBER(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(negate_number) {
    RETURN(lk_number_new(VM, -CNUMBER(self)));
}
LK_LIB_DEFINECFUNC(round_number) {
    double f = CNUMBER(self), i = floor(f);
    RETURN(lk_number_new(VM, f - i > 0.5 ? i + 1 : f - i < -0.5 ? i - 1 : i));
}
LK_LIB_DEFINECFUNC(subtract_number_number) {
    RETURN(lk_number_new(VM, CNUMBER(self) - CNUMBER(ARG(0))));
}
LK_LIB_DEFINECFUNC(subtractB_number_number) {
    CNUMBER(self) -= CNUMBER(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(to_string_number_number_number) {
    int i = CNUMBER(ARG(0)), f = CNUMBER(ARG(1));
    char cString[200];
    snprintf(cString, 200, "%*.*f", i, f, CNUMBER(self));
    RETURN(lk_string_newFromCString(VM, cString));
}
LK_LIB_DEFINEINIT(lk_number_libInit) {
    lk_object_t *number = vm->t_number;
    lk_lib_setGlobal("Number", number);
    lk_lib_setCFunc(number, "abs", abs_number, NULL);
    lk_lib_setCFunc(number, "+", add_number_number, number, NULL);
    lk_lib_setCFunc(number, "+=", addB_number_number, number, NULL);
    lk_lib_setCFunc(number, "ceil", ceil_number, NULL);
    lk_lib_setCFunc(number, "<=>", subtract_number_number, number, NULL);
    lk_lib_setCFunc(number, "%", divide_number_number, number, NULL);
    lk_lib_setCFunc(number, "%=", divideB_number_number, number, NULL);
    lk_lib_setCFunc(number, "==", eq_number_number, number, NULL);
    lk_lib_setCFunc(number, "floor", floor_number, NULL);
    lk_lib_setCFunc(number, ">", gt_number_number, number, NULL);
    lk_lib_setCFunc(number, "integer?", integerQuestion_number, NULL);
    lk_lib_setCFunc(number, "<", lt_number_number, number, NULL);
    lk_lib_setCFunc(number, "mod", mod_number_number, number, NULL);
    lk_lib_setCFunc(number, "mod!", modB_number_number, number, NULL);
    lk_lib_setCFunc(number, "*", multiply_number_number, number, NULL);
    lk_lib_setCFunc(number, "*=", multiplyB_number_number, number, NULL);
    lk_lib_setCFunc(number, "negate", negate_number, NULL);
    lk_lib_setCFunc(number, "round", round_number, NULL);
    lk_lib_setCFunc(number, "-", subtract_number_number, number, NULL);
    lk_lib_setCFunc(number, "-=", subtractB_number_number, number, NULL);
    lk_lib_setCFunc(number, "toString", to_string_number_number_number, number, number, NULL);
}

/* new */
lk_number_t *lk_number_new(lk_vm_t *vm, double number) {
    lk_number_t *self = LK_NUMBER(lk_object_alloc(vm->t_number));
    CNUMBER(self) = number;
    return self;
}
