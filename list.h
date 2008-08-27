#ifndef LK_LIST_H
#define LK_LIST_H
#include "types.h"

/* type */
void lk_list_typeinit(lk_vm_t *vm);
void lk_list_libinit(lk_vm_t *vm);

/* new */
lk_list_t *lk_list_new(lk_vm_t *vm);
lk_list_t *lk_list_new_fromdarray(lk_vm_t *vm, darray_t *from);
lk_list_t *lk_list_newfromargv(lk_vm_t *vm, int argc, const char **argv);

/* update */
void lk_list_insert_num_obj(lk_list_t *self, lk_num_t *index, lk_obj_t *value);
void lk_list_remove_num(lk_list_t *self, lk_num_t *index);
void lk_list_set_num_obj(lk_list_t *self, lk_num_t *index, lk_obj_t *value);
void lk_list_set_num_num_list(lk_list_t *self, lk_num_t *from, lk_num_t *to, lk_list_t *list);

/* info */
lk_obj_t *lk_list_at_num(lk_list_t *self, lk_num_t *index);
void lk_list_flatten(lk_obj_t *self, lk_scope_t *local);
#endif
