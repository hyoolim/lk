#ifndef LK_MAP_H
#define LK_MAP_H

/* type */
typedef struct lk_gset lk_map_t;
#include "vm.h"
#include "gset.h"
#define LK_MAP(v) ((lk_map_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_map_extinittypes);
LK_EXT_DEFINIT(lk_map_extinitfuncs);

/* new */
lk_map_t *lk_map_new(lk_vm_t *vm);
lk_map_t *lk_map_newfrompt(lk_vm_t *vm, set_t *ht);

/* update */
void lk_map_set(lk_map_t *self, lk_obj_t *k, lk_obj_t *v);
void lk_map_setbycstr(lk_map_t *self, const char *k, lk_obj_t *v);

/* info */
lk_obj_t *lk_map_get(lk_map_t *self, lk_obj_t *k);
lk_obj_t *lk_map_getbycstr(lk_map_t *self, const char *k);
#endif
