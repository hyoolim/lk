#ifndef LK_EXT_H
#define LK_EXT_H
#include "vm.h"
#include "error.h"
#include "func.h"
#include "number.h"
#include "gc.h"
#include "object.h"

/* type */
typedef struct lk_library {
    struct lk_common o;
    void             *lib;
} lk_library_t;
#define LK_EXT(v) ((lk_library_t *)(v))

/* ext shortcuts */
#define ARG(i) (assert(local != NULL && 0 <= (i) && (i) < local->argc),  LK_OBJ(LIST_ATPTR(&local->stack, (i))))
#define DONE return
#define RETURN(v) \
    do { \
        lk_object_t *_r = LK_OBJ(v); \
        assert(_r != NULL); \
        lk_scope_stackpush(local->caller, _r); \
        DONE; \
    } while(0)
#define GOTO(name) (name(self, local))

#define CHAR(v) (LK_CHAR(v)->data)
#define CHARSET(v) (&LK_CHARSET(v)->data)
#define CNUMBER(v) (LK_NUMBER(v)->data)
#define CSIZE(v) ((int)CNUMBER(v))
#define CSTRING(v) (darray_toCString(DARRAY(v)))
#define DARRAY(v) (&LK_SEQ(v)->data)
#define FALSE (VM->t_false)
#define NIL (VM->t_nil)
#define QPHASH(v) (&LK_MAP(v)->data)
#define TRUE (VM->t_true)
#define VM (LK_VM(self))

#define CHKOPT(v, o) ((v) & (o))
#define SETOPT(v, o) ((v) |= (o))
#define CLROPT(v, o) ((v) &= ~(o))

/* ext map */
void lk_library_extinit(lk_vm_t *vm);

/* update */
void lk_lib_setObject(lk_object_t *parent, const char *k, lk_object_t *v);
void lk_lib_setGlobal(const char *k, lk_object_t *v);
void lk_lib_setCField(lk_object_t *self, const char *k, lk_object_t *t, size_t offset);
#endif
