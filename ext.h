#ifndef LK_EXT_H
#define LK_EXT_H
#include "vm.h"
#include "error.h"
#include "func.h"
#include "fixnum.h"
#include "gc.h"
#include "obj.h"

/* type */
typedef struct lk_library {
    struct lk_common o;
    void             *lib;
} lk_library_t;
#define LK_EXT(v) ((lk_library_t *)(v))

/* ext shortcuts */
#define ARG(i) ( \
assert(env != NULL && 0 <= (i) && (i) < env->argc), \
LK_OBJ(LIST_ATPTR(&env->stack, (i))))
#define DONE return
#define RETURN(v) do { \
    lk_object_t *_r = LK_OBJ(v); \
    assert(_r != NULL); \
    lk_frame_stackpush(env->caller, _r); \
    DONE; \
} while(0)
#define GOTO(name) (name(self, env))

#define CHAR(v) (LK_CHAR(v)->data)
#define CHARSET(v) (&LK_CHARSET(v)->data)
#define CSTRING(v) (darray_toCString(DARRAY(v)))
#define DARRAY(v) (&LK_SEQ(v)->data)
#define DOUBLE(v) (LK_FR(v)->data)
#define FALSE (VM->t_false)
#define INT(v) (LK_FI(v)->data)
#define NIL (VM->t_nil)
#define QPHASH(v) (&LK_MAP(v)->data)
#define TRUE (VM->t_true)
#define VM (LK_VM(self))

#define CHKOPT(v, o) ((v) & (o))
#define SETOPT(v, o) ((v) |= (o))
#define CLROPT(v, o) ((v) &= ~(o))

/* ext map */
LK_EXT_DEFINIT(lk_library_extinit);

/* update */
void lk_lib_setObject(lk_object_t *parent, const char *k, lk_object_t *v);
void lk_lib_setGlobal(const char *k, lk_object_t *v);
void lk_lib_setCField(lk_object_t *self, const char *k, lk_object_t *t, size_t offset);
void lk_lib_setCFunc(lk_object_t *obj, const char *k, lk_cfuncfunc_t *func, ...);
#endif
