#ifndef LK_VECTOR_H
#define LK_VECTOR_H

/* type */
typedef struct lk_seq lk_vector_t;
#define LK_VECTOR(v) ((lk_vector_t *)(v))
#include "vm.h"
#include "seq.h"

/* ext map */
LK_LIB_DEFINEINIT(lk_vector_libPreInit);
LK_LIB_DEFINEINIT(lk_vector_libInit);
#endif
