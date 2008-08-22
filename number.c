#include "number.h"
#include "ext.h"
#include "string.h"
#include <math.h>

/* ext map - types */
static void alloc_number(lk_object_t *self, lk_object_t *parent) {
    CNUMBER(self) = CNUMBER(parent);
}
void lk_number_libPreInit(lk_vm_t *vm) {
    vm->t_number = lk_object_allocWithSize(vm->t_object, sizeof(lk_number_t));
    lk_object_setallocfunc(vm->t_number, alloc_number);
}

/* ext map - funcs */
static void abs_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_number_new(VM, fabs(CNUMBER(self))));
}
static void add_number_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_number_new(VM, CNUMBER(self) + CNUMBER(ARG(0))));
}
static void addB_number_number(lk_object_t *self, lk_scope_t *local) {
    CNUMBER(self) += CNUMBER(ARG(0));
    RETURN(self);
}
static void ceil_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_number_new(VM, ceil(CNUMBER(self))));
}
static void divide_number_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_number_new(VM, CNUMBER(self) / CNUMBER(ARG(0))));
}
static void divideB_number_number(lk_object_t *self, lk_scope_t *local) {
    CNUMBER(self) /= CNUMBER(ARG(0));
    RETURN(self);
}
static void eq_number_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(CNUMBER(self) == CNUMBER(ARG(0)) ? TRUE : FALSE);
}
static void floor_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_number_new(VM, floor(CNUMBER(self))));
}
static void gt_number_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(CNUMBER(self) > CNUMBER(ARG(0)) ? TRUE : FALSE);
}
static void integerQuestion_number(lk_object_t *self, lk_scope_t *local) {
    double i;
    RETURN(modf(CNUMBER(self), &i) == 0.0 ? TRUE : FALSE);
}
static void lt_number_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(CNUMBER(self) < CNUMBER(ARG(0)) ? TRUE : FALSE);
}
static void mod_number_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_number_new(VM, fmod(CNUMBER(self), CNUMBER(ARG(0)))));
}
static void modB_number_number(lk_object_t *self, lk_scope_t *local) {
    CNUMBER(self) = fmod(CNUMBER(self), CNUMBER(ARG(0)));
    RETURN(self);
}
static void multiply_number_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_number_new(VM, CNUMBER(self) * CNUMBER(ARG(0))));
}
static void multiplyB_number_number(lk_object_t *self, lk_scope_t *local) {
    CNUMBER(self) *= CNUMBER(ARG(0));
    RETURN(self);
}
static void negate_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_number_new(VM, -CNUMBER(self)));
}
static void round_number(lk_object_t *self, lk_scope_t *local) {
    double f = CNUMBER(self), i = floor(f);
    RETURN(lk_number_new(VM, f - i > 0.5 ? i + 1 : f - i < -0.5 ? i - 1 : i));
}
static void subtract_number_number(lk_object_t *self, lk_scope_t *local) {
    RETURN(lk_number_new(VM, CNUMBER(self) - CNUMBER(ARG(0))));
}
static void subtractB_number_number(lk_object_t *self, lk_scope_t *local) {
    CNUMBER(self) -= CNUMBER(ARG(0));
    RETURN(self);
}
static void to_string_number_number_number(lk_object_t *self, lk_scope_t *local) {
    int i = CNUMBER(ARG(0)), f = CNUMBER(ARG(1));
    char cString[200];
    snprintf(cString, 200, "%*.*f", i, f, CNUMBER(self));
    RETURN(lk_string_newFromCString(VM, cString));
}
void lk_number_libInit(lk_vm_t *vm) {
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
