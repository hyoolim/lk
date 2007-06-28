#ifndef LK_DICT_H
#define LK_DICT_H

/* type */
typedef struct lk_gset lk_dict_t;
#include "vm.h"
#include "gset.h"
#define LK_DICT(v) ((lk_dict_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_dict_extinittypes);
LK_EXT_DEFINIT(lk_dict_extinitfuncs);

/* new */
lk_dict_t *lk_dict_new(lk_vm_t *vm);
lk_dict_t *lk_dict_newfrompt(lk_vm_t *vm, set_t *ht);

/* update */
void lk_dict_set(lk_dict_t *self, lk_obj_t *k, lk_obj_t *v);
void lk_dict_setbycstr(lk_dict_t *self, const char *k, lk_obj_t *v);

/* info */
lk_obj_t *lk_dict_get(lk_dict_t *self, lk_obj_t *k);
lk_obj_t *lk_dict_getbycstr(lk_dict_t *self, const char *k);
#endif
