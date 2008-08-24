#ifndef LK_VECTOR_H
#define LK_VECTOR_H

/* type */
typedef struct lk_seq lk_vector_t;
#define LK_VECTOR(v) ((lk_vector_t *)(v))
#include "vm.h"
#include "seq.h"

/* ext map */
void lk_vector_typeinit(lk_vm_t *vm);
void lk_vector_libinit(lk_vm_t *vm);
#endif
