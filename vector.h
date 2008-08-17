#ifndef LK_VECTOR_H
#define LK_VECTOR_H

/* type */
typedef struct lk_seq lk_vector_t;
#define LK_VECTOR(v) ((lk_vector_t *)(v))
#include "vm.h"
#include "seq.h"

/* ext map */
LK_EXT_DEFINIT(lk_vector_extinittypes);
LK_EXT_DEFINIT(lk_vector_extinitfuncs);
#endif
