#include "ext.h"
#include <math.h>

/* ext map - types */
static void alloc_num(lk_obj_t *self, lk_obj_t *parent) {
    CNUMBER(self) = CNUMBER(parent);
}
void lk_num_typeinit(lk_vm_t *vm) {
    vm->t_num = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_num_t));
    lk_obj_setallocfunc(vm->t_num, alloc_num);
}

/* ext map - funcs */
static void abs_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, fabs(CNUMBER(self))));
}
static void add_num_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, CNUMBER(self) + CNUMBER(ARG(0))));
}
static void addB_num_num(lk_obj_t *self, lk_scope_t *local) {
    CNUMBER(self) += CNUMBER(ARG(0));
    RETURN(self);
}
static void ceil_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, ceil(CNUMBER(self))));
}
static void divide_num_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, CNUMBER(self) / CNUMBER(ARG(0))));
}
static void divideB_num_num(lk_obj_t *self, lk_scope_t *local) {
    CNUMBER(self) /= CNUMBER(ARG(0));
    RETURN(self);
}
static void eq_num_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(CNUMBER(self) == CNUMBER(ARG(0)) ? TRUE : FALSE);
}
static void floor_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, floor(CNUMBER(self))));
}
static void gt_num_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(CNUMBER(self) > CNUMBER(ARG(0)) ? TRUE : FALSE);
}
static void integerQuestion_num(lk_obj_t *self, lk_scope_t *local) {
    double i;
    RETURN(modf(CNUMBER(self), &i) == 0.0 ? TRUE : FALSE);
}
static void lt_num_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(CNUMBER(self) < CNUMBER(ARG(0)) ? TRUE : FALSE);
}
static void mod_num_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, fmod(CNUMBER(self), CNUMBER(ARG(0)))));
}
static void modB_num_num(lk_obj_t *self, lk_scope_t *local) {
    CNUMBER(self) = fmod(CNUMBER(self), CNUMBER(ARG(0)));
    RETURN(self);
}
static void multiply_num_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, CNUMBER(self) * CNUMBER(ARG(0))));
}
static void multiplyB_num_num(lk_obj_t *self, lk_scope_t *local) {
    CNUMBER(self) *= CNUMBER(ARG(0));
    RETURN(self);
}
static void negate_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, -CNUMBER(self)));
}
static void round_num(lk_obj_t *self, lk_scope_t *local) {
    double f = CNUMBER(self), i = floor(f);
    RETURN(lk_num_new(VM, f - i > 0.5 ? i + 1 : f - i < -0.5 ? i - 1 : i));
}
static void subtract_num_num(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_num_new(VM, CNUMBER(self) - CNUMBER(ARG(0))));
}
static void subtractB_num_num(lk_obj_t *self, lk_scope_t *local) {
    CNUMBER(self) -= CNUMBER(ARG(0));
    RETURN(self);
}
static void to_str_num_num_num(lk_obj_t *self, lk_scope_t *local) {
    int i = CNUMBER(ARG(0)), f = CNUMBER(ARG(1));
    char cstr[200];
    snprintf(cstr, 200, "%*.*f", i, f, CNUMBER(self));
    RETURN(lk_str_new_fromcstr(VM, cstr));
}
void lk_num_libinit(lk_vm_t *vm) {
    lk_obj_t *num = vm->t_num;
    lk_global_set("Number", num);
    lk_obj_set_cfunc_lk(num, "abs", abs_num, NULL);
    lk_obj_set_cfunc_lk(num, "+", add_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "+=", addB_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "ceil", ceil_num, NULL);
    lk_obj_set_cfunc_lk(num, "<=>", subtract_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "%", divide_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "%=", divideB_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "==", eq_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "floor", floor_num, NULL);
    lk_obj_set_cfunc_lk(num, ">", gt_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "integer?", integerQuestion_num, NULL);
    lk_obj_set_cfunc_lk(num, "<", lt_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "mod", mod_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "mod!", modB_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "*", multiply_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "*=", multiplyB_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "negate", negate_num, NULL);
    lk_obj_set_cfunc_lk(num, "round", round_num, NULL);
    lk_obj_set_cfunc_lk(num, "-", subtract_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "-=", subtractB_num_num, num, NULL);
    lk_obj_set_cfunc_lk(num, "toString", to_str_num_num_num, num, num, NULL);
}

/* new */
lk_num_t *lk_num_new(lk_vm_t *vm, double num) {
    lk_num_t *self = LK_NUMBER(lk_obj_alloc(vm->t_num));
    CNUMBER(self) = num;
    return self;
}
