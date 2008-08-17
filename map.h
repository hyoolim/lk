#ifndef LK_MAP_H
#define LK_MAP_H

/* type */
typedef struct lk_gset lk_map_t;
#define LK_MAP(v) ((lk_map_t *)(v))
#include "vm.h"
#include "gset.h"

/* ext map */
LK_EXT_DEFINIT(lk_map_extinittypes);
LK_EXT_DEFINIT(lk_map_extinitfuncs);

/* new */
lk_map_t *lk_map_new(lk_vm_t *vm);
lk_map_t *lk_map_newfrompt(lk_vm_t *vm, set_t *ht);

/* update */
void lk_map_set(lk_map_t *self, lk_object_t *k, lk_object_t *v);
void lk_map_setbycstr(lk_map_t *self, const char *k, lk_object_t *v);

/* info */
lk_object_t *lk_map_get(lk_map_t *self, lk_object_t *k);
lk_object_t *lk_map_getbycstr(lk_map_t *self, const char *k);
#endif
