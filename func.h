#ifndef LK_FUNC_H
#define LK_FUNC_H

/* type */
typedef struct lk_Func lk_Func_t;
typedef struct lk_Cfunc lk_Cfunc_t;
typedef struct lk_Gfunc lk_Gfunc_t;
typedef struct lk_Kfunc lk_Kfunc_t;
typedef struct lk_Sig lk_Sig_t;
#include "vm.h"
#include "frame.h"
#include "instr.h"
struct lk_Func {
    struct lk_Common obj;
    struct lk_Commonfunc {
        lk_Instr_t  *sigdef;
        int          minargc;
        int          maxargc;
        Sequence_t   *sigs;
        lk_Sig_t    *rest;
        lk_String_t *doc;
        uint8_t      opts;
    }                cf;
};
#define LK_FUNC(v) ((lk_Func_t *)(v))
#define LK_FUNCORUNNING  (1 << 0)
#define LK_FUNCOASSIGNED (1 << 1)
struct lk_Cfunc {
    struct lk_Common      obj;
    struct lk_Commonfunc cf;
    lk_Cfuncfunc_t       *func;
};
#define LK_CFUNC(v) ((lk_Cfunc_t *)(v))
struct lk_Gfunc {
    struct lk_Common      obj;
    struct lk_Commonfunc  cf;
    Sequence_t            *funcs;
};
#define LK_GFUNC(v) ((lk_Gfunc_t *)(v))
struct lk_Kfunc {
    struct lk_Common      obj;
    struct lk_Commonfunc  cf;
    lk_Frame_t           *frame;
    lk_Instr_t           *first;
};
#define LK_KFUNC(v) ((lk_Kfunc_t *)(v))
struct lk_Sig {
    struct lk_Common  obj;
    lk_String_t      *name;
    lk_Object_t      *check;
    uint8_t           isself;
};
#define LK_SIG(v) ((lk_Sig_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Func_extinittypes);
LK_EXT_DEFINIT(lk_Func_extinitfuncs);

/* new */
lk_Cfunc_t *lk_Cfunc_new(lk_Vm_t * vm, lk_Cfuncfunc_t *func, int minargc, int maxargc);
lk_Kfunc_t *lk_Kfunc_new(lk_Vm_t *vm, lk_Frame_t *frame, lk_Instr_t *first);
lk_Gfunc_t *lk_Gfunc_new(lk_Vm_t *vm);
lk_Sig_t *lk_Sig_new(lk_Vm_t *vm, lk_String_t *name, lk_Object_t *type);

/* update */
lk_Gfunc_t *lk_Func_combine(lk_Func_t *self, lk_Func_t *other);
lk_Func_t *lk_Func_match(lk_Func_t *self, lk_Frame_t *args, lk_Object_t *recv);
void lk_Kfunc_updatesig(lk_Kfunc_t *self);
#endif
