#ifndef LK_MAP_H
#define LK_MAP_H

/* type */
typedef struct lk_Gset lk_Map_t;
#include "vm.h"
#include "gset.h"
#define LK_MAP(v) ((lk_Map_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Map_extinittypes);
LK_EXT_DEFINIT(lk_Map_extinitfuncs);

/* new */
lk_Map_t *lk_Map_new(lk_Vm_t *vm);
lk_Map_t *lk_Map_newfrompt(lk_Vm_t *vm, set_t *ht);

/* update */
void lk_Map_set(lk_Map_t *self, lk_Object_t *k, lk_Object_t *v);
void lk_Map_setbycstr(lk_Map_t *self, const char *k, lk_Object_t *v);

/* info */
lk_Object_t *lk_Map_get(lk_Map_t *self, lk_Object_t *k);
lk_Object_t *lk_Map_getbycstr(lk_Map_t *self, const char *k);
#endif
