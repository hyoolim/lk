#ifndef LK_SEQ_H
#define LK_SEQ_H

/* type */
typedef struct lk_seq lk_seq_t;
#define LK_SEQ(v) ((lk_seq_t *)(v))
#include "vm.h"
struct lk_seq {
    struct lk_common o;
    darray_t        data;
};

/* ext map */
void lk_seq_libPreInit(lk_vm_t *vm);
void lk_seq_libInit(lk_vm_t *vm);
#endif
