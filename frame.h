#ifndef LK_FRAME_H
#define LK_FRAME_H

/* type */
typedef struct lk_frame lk_frame_t;
#define LK_FRAME(v) ((lk_frame_t *)(v))
#include "vm.h"
#include "instr.h"
#include "list.h"
struct lk_frame {
    struct lk_common o;
    enum lk_frametype_t {
        LK_FRAMETYPE_APPLY = 1,
        LK_FRAMETYPE_LIST,
        LK_FRAMETYPE_RETURN
    }                    type;
    darray_t            stack;
    lk_frame_t          *frame;
    lk_object_t         *receiver;
    lk_object_t         *self;
    lk_frame_t          *caller;
    lk_frame_t          *child;
    lk_frame_t          *returnto;
    lk_instr_t          *first;
    lk_instr_t          *next;
    lk_instr_t          *current;
    lk_object_t         *func;
    struct lk_slot      *lastslot;
    int                  argc;
};

/* ext map */
LK_LIB_DEFINEINIT(lk_frame_libPreInit);
LK_LIB_DEFINEINIT(lk_frame_libInit);

/* update */
lk_frame_t *lk_frame_new(lk_vm_t *vm);
void lk_frame_stackpush(lk_frame_t *self, lk_object_t *v);
lk_object_t *lk_frame_stackpop(lk_frame_t *self);
lk_object_t *lk_frame_stackpeek(lk_frame_t *self);
lk_list_t *lk_frame_stacktolist(lk_frame_t *self);
#endif
