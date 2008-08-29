#ifndef LK_LIB_H
#define LK_LIB_H
#include "types.h"
#include "bool.h"
#include "charset.h"
#include "dl.h"
#include "err.h"
#include "file.h"
#include "func.h"
#include "instr.h"
#include "map.h"
#include "obj.h"
#include "rand.h"
#include "seq.h"
#include "str.h"
#include "vec.h"
#include "char.h"
#include "env.h"
#include "lib.h"
#include "dir.h"
#include "gc.h"
#include "list.h"
#include "num.h"
#include "parser.h"
#include "pipe.h"
#include "scope.h"
#include "socket.h"
#include "vm.h"

/* ext shortcuts */
#define ARG(i) (assert(local != NULL && 0 <= (i) && (i) < local->argc),  LK_OBJ(LIST_ATPTR(&local->stack, (i))))
#define DONE return
#define RETURN(v) \
    do { \
        lk_obj_t *_r = LK_OBJ(v); \
        assert(_r != NULL); \
        lk_scope_stackpush(local->caller, _r); \
        DONE; \
    } while(0)
#define GOTO(name) (name(self, local))

#define CHAR(v) (LK_CHAR(v)->data)
#define CHARSET(v) (&LK_CHARSET(v)->data)
#define CNUMBER(v) (LK_NUMBER(v)->data)
#define CSIZE(v) ((int)CNUMBER(v))
#define CSTRING(v) (darray_tocstr(DARRAY(v)))
#define DARRAY(v) (&LK_SEQ(v)->data)
#define FALSE (VM->t_false)
#define NIL (VM->t_nil)
#define QPHASH(v) (&LK_MAP(v)->data)
#define TRUE (VM->t_true)
#define VM (LK_VM(self))

#define CHKOPT(v, o) ((v) & (o))
#define SETOPT(v, o) ((v) |= (o))
#define CLROPT(v, o) ((v) &= ~(o))
#endif
