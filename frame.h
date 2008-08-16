#ifndef LK_FRAME_H
#define LK_FRAME_H

/* type */
typedef struct lk_Frame lk_Frame_t;
#include "vm.h"
#include "instr.h"
#include "list.h"
struct lk_Frame {
    struct lk_Common     obj;
    enum lk_Frametype_t {
        LK_FRAMETYPE_APPLY = 1,
        LK_FRAMETYPE_LIST,
        LK_FRAMETYPE_RETURN
    }                    type;
    Sequence_t            stack;
    lk_Frame_t          *frame;
    lk_Object_t         *receiver;
    lk_Object_t         *self;
    lk_Frame_t          *caller;
    lk_Frame_t          *child;
    lk_Frame_t          *returnto;
    lk_Instr_t          *first;
    lk_Instr_t          *next;
    lk_Instr_t          *current;
    lk_Object_t         *func;
    struct lk_Slot      *lastslot;
    int                  argc;
};
#define LK_FRAME(v) ((lk_Frame_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Frame_extinittypes);
LK_EXT_DEFINIT(lk_Frame_extinitfuncs);

/* update */
lk_Frame_t *lk_Frame_new(lk_Vm_t *vm);
void lk_Frame_stackpush(lk_Frame_t *self, lk_Object_t *v);
lk_Object_t *lk_Frame_stackpop(lk_Frame_t *self);
lk_Object_t *lk_Frame_stackpeek(lk_Frame_t *self);
lk_List_t *lk_Frame_stacktolist(lk_Frame_t *self);
#endif
