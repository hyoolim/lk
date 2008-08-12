#ifndef LK_EXT_H
#define LK_EXT_H
#include "vm.h"
#include "error.h"
#include "func.h"
#include "fixnum.h"
#include "gc.h"
#include "obj.h"

/* type */
typedef struct lk_ext {
    struct lk_common  obj;
    void             *lib;
} lk_ext_t;
#define LK_EXT(v) ((lk_ext_t *)(v))

/* ext shortcuts */
#define ARG(i) ( \
assert(env != NULL && 0 <= (i) && (i) < env->argc), \
LK_OBJ(LIST_ATPTR(&env->stack, (i))))
#define DONE return
#define RETURN(v) do { \
    lk_obj_t *_r = LK_OBJ(v); \
    assert(_r != NULL); \
    lk_frame_stackpush(env->caller, _r); \
    DONE; \
} while(0)
#define GOTO(name) (name(self, env))

#define CHAR(v) (LK_CHAR(v)->c)
#define CSET(v) (&LK_CSET(v)->cs)
#define DBL(v) (LK_FR(v)->r)
#define F (VM->t_false)
#define INT(v) (LK_FI(v)->i)
#define LIST(v) (&LK_GLIST(v)->data)
#define CSTR(v) (list_tocstr(LIST(v)))
#define N (VM->t_nil)
#define T (VM->t_true)
#define VM (LK_VM(self))

#define CHKOPT(v, o) ((v) & (o))
#define SETOPT(v, o) ((v) |= (o))
#define CLROPT(v, o) ((v) &= ~(o))

/* ext map */
LK_EXT_DEFINIT(lk_ext_extinit);

/* update */
void lk_ext_set(lk_obj_t *proto, const char *k, lk_obj_t *v);
void lk_ext_global(const char *k, lk_obj_t *v);
void lk_ext_cfield(lk_obj_t *self, const char *k, lk_obj_t *t,
                   size_t offset);
void lk_ext_cfunc(lk_obj_t *obj, const char *k, lk_cfuncfunc_t *func, ...);
#endif
