#ifndef LK_MAP_H
#define LK_MAP_H
#include "types.h"

// type
struct lk_map {
    struct lk_common o;
    qphash_t data;
};

// ext map
void lk_map_type_init(lk_vm_t *vm);
void lk_map_lib_init(lk_vm_t *vm);

// new
lk_map_t *lk_map_new(lk_vm_t *vm);

// update
void lk_map_clear(lk_map_t *self);
void lk_map_set(lk_map_t *self, lk_obj_t *k, lk_obj_t *v);
void lk_map_set_str_obj(lk_map_t *self, lk_str_t *key, lk_obj_t *value);
void lk_map_setWithCStringKey(lk_map_t *self, const char *k, lk_obj_t *v);

// info
lk_obj_t *lk_map_at_str(lk_map_t *self, lk_str_t *key);
lk_obj_t *lk_map_get(lk_map_t *self, lk_obj_t *k);
lk_obj_t *lk_map_getByCStringKey(lk_map_t *self, const char *k);
lk_list_t *lk_map_keys(lk_map_t *self);
lk_num_t *lk_map_size(lk_map_t *self);
lk_list_t *lk_map_values(lk_map_t *self);
#endif
