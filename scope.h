#ifndef LK_SCOPE_H
#define LK_SCOPE_H

/* type */
typedef struct lk_scope lk_scope_t;
#define LK_SCOPE(v) ((lk_scope_t *)(v))
#include "vm.h"
#include "instr.h"
#include "list.h"
struct lk_scope {
    struct lk_common o;
    enum lk_scopetype_t {
        LK_SCOPETYPE_APPLY = 1,
        LK_SCOPETYPE_LIST,
        LK_SCOPETYPE_RETURN
    }                    type;
    darray_t            stack;
    lk_scope_t          *scope;
    lk_object_t         *receiver;
    lk_object_t         *self;
    lk_scope_t          *caller;
    lk_scope_t          *child;
    lk_scope_t          *returnto;
    lk_instr_t          *first;
    lk_instr_t          *next;
    lk_instr_t          *current;
    lk_object_t         *func;
    struct lk_slot      *lastslot;
    int                  argc;
};

/* ext map */
LK_LIB_DEFINEINIT(lk_scope_libPreInit);
LK_LIB_DEFINEINIT(lk_scope_libInit);

/* update */
lk_scope_t *lk_scope_new(lk_vm_t *vm);
void lk_scope_stackpush(lk_scope_t *self, lk_object_t *v);
lk_object_t *lk_scope_stackpop(lk_scope_t *self);
lk_object_t *lk_scope_stackpeek(lk_scope_t *self);
lk_list_t *lk_scope_stacktolist(lk_scope_t *self);
#endif
