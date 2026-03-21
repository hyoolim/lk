#ifndef LK_SCOPE_H
#define LK_SCOPE_H
#include "types.h"

// type
struct lk_scope {
    struct lk_common o;
    enum lk_scopetype_t { LK_SCOPETYPE_APPLY = 1, LK_SCOPETYPE_LIST, LK_SCOPETYPE_RETURN } type;
    vec_t stack;
    lk_scope_t *scope;
    lk_obj_t *receiver;
    lk_obj_t *self;
    lk_scope_t *caller;
    lk_scope_t *child;
    lk_scope_t *returnto;
    lk_instr_t *first;
    lk_instr_t *next;
    lk_instr_t *current;
    lk_obj_t *func;
    struct lk_slot *lastslot;
    int argc;
};

// ext map
void lk_scope_type_init(lk_vm_t *vm);
void lk_scope_lib_init(lk_vm_t *vm);

// update
lk_scope_t *lk_scope_new(lk_vm_t *vm);
void lk_scope_stack_push(lk_scope_t *self, lk_obj_t *v);
lk_obj_t *lk_scope_stack_pop(lk_scope_t *self);
lk_obj_t *lk_scope_stack_peek(lk_scope_t *self);
lk_list_t *lk_scope_stack_to_list(lk_scope_t *self);
#endif
