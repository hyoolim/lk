#ifndef LK_SCOPE_H
#define LK_SCOPE_H
#include "types.h"

/* type */
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
void lk_scope_typeinit(lk_vm_t *vm);
void lk_scope_libinit(lk_vm_t *vm);

/* update */
lk_scope_t *lk_scope_new(lk_vm_t *vm);
void lk_scope_stackpush(lk_scope_t *self, lk_object_t *v);
lk_object_t *lk_scope_stackpop(lk_scope_t *self);
lk_object_t *lk_scope_stackpeek(lk_scope_t *self);
lk_list_t *lk_scope_stacktolist(lk_scope_t *self);
#endif
