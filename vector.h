#ifndef LK_VECTOR_H
#define LK_VECTOR_H

/* type */
typedef struct lk_Glist lk_Vector_t;
#include "vm.h"
#include "glist.h"
#define LK_VECTOR(v) ((lk_Vector_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Vector_extinittypes);
LK_EXT_DEFINIT(lk_Vector_extinitfuncs);
#endif
