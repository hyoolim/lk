#ifndef LK_FUNC_H
#define LK_FUNC_H

/* type */
typedef struct lk_func lk_func_t;
typedef struct lk_gfunc lk_gfunc_t;
typedef struct lk_kfunc lk_kfunc_t;
typedef struct lk_sig lk_sig_t;
#include "vm.h"
#include "scope.h"
#include "instr.h"
struct lk_func {
    struct lk_common o;
    struct lk_commonfunc {
        lk_instr_t  *sigdef;
        int          minargc;
        int          maxargc;
        darray_t   *sigs;
        lk_sig_t    *rest;
        lk_string_t *doc;
        uint8_t      opts;
    }                cf;
};
#define LK_FUNC(v) ((lk_func_t *)(v))
#define LK_FUNCORUNNING  (1 << 0)
#define LK_FUNCOASSIGNED (1 << 1)
struct lk_cfunc {
    struct lk_common o;
    struct lk_commonfunc cf;
    enum {
        LK_CFUNC_CC_LK,
        LK_CFUNC_CC_CRETURN,
        LK_CFUNC_CC_CVOID
    } cc;
    union {
        lk_cfunc_lk_t *lk;
        lk_cfunc_r0_t *r0;
        lk_cfunc_r1_t *r1;
        lk_cfunc_r2_t *r2;
        lk_cfunc_r3_t *r3;
        lk_cfunc_v0_t *v0;
        lk_cfunc_v1_t *v1;
        lk_cfunc_v2_t *v2;
        lk_cfunc_v3_t *v3;
    } cfunc;
};
#define LK_CFUNC(v) ((lk_cfunc_t *)(v))
struct lk_gfunc {
    struct lk_common o;
    struct lk_commonfunc  cf;
    darray_t            *funcs;
};
#define LK_GFUNC(v) ((lk_gfunc_t *)(v))
struct lk_kfunc {
    struct lk_common o;
    struct lk_commonfunc  cf;
    lk_scope_t           *scope;
    lk_instr_t           *first;
};
#define LK_KFUNC(v) ((lk_kfunc_t *)(v))
struct lk_sig {
    struct lk_common o;
    lk_string_t      *name;
    lk_object_t      *check;
    uint8_t           isself;
};
#define LK_SIG(v) ((lk_sig_t *)(v))

/* ext map */
void lk_func_typeinit(lk_vm_t *vm);
void lk_func_libinit(lk_vm_t *vm);

/* new */
lk_cfunc_t *lk_cfunc_new(lk_vm_t * vm, lk_cfuncfunc_t *func, int minargc, int maxargc);
lk_kfunc_t *lk_kfunc_new(lk_vm_t *vm, lk_scope_t *scope, lk_instr_t *first);
lk_gfunc_t *lk_gfunc_new(lk_vm_t *vm);
lk_sig_t *lk_sig_new(lk_vm_t *vm, lk_string_t *name, lk_object_t *type);

/* update */
lk_gfunc_t *lk_func_combine(lk_func_t *self, lk_func_t *other);
lk_func_t *lk_func_match(lk_func_t *self, lk_scope_t *args, lk_object_t *recv);
void lk_kfunc_updatesig(lk_kfunc_t *self);
#endif
