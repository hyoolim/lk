#ifndef LK_SEQ_H
#define LK_SEQ_H
#include "types.h"

/* type */
struct lk_seq {
    struct lk_common o;
    darray_t        data;
};

/* ext map */
void lk_seq_typeinit(lk_vm_t *vm);
void lk_seq_libinit(lk_vm_t *vm);
#endif
