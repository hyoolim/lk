#ifndef LK_VECTOR_H
#define LK_VECTOR_H

/* type */
typedef struct lk_glist lk_vector_t;
#include "vm.h"
#include "glist.h"
#define LK_VECTOR(v) ((lk_vector_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_vector_extinittypes);
LK_EXT_DEFINIT(lk_vector_extinitfuncs);
#endif
