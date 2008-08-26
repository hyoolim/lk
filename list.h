#ifndef LK_LIST_H
#define LK_LIST_H
#include "types.h"

/* type */
void lk_list_typeinit(lk_vm_t *vm);
void lk_list_libinit(lk_vm_t *vm);

/* new */
lk_list_t *lk_list_new(lk_vm_t *vm);
lk_list_t *lk_list_newFromDArray(lk_vm_t *vm, darray_t *from);
lk_list_t *lk_list_newfromargv(lk_vm_t *vm, int argc, const char **argv);

/* update */
void lk_list_insert_number_object(lk_list_t *self, lk_number_t *index, lk_object_t *value);
void lk_list_remove_number(lk_list_t *self, lk_number_t *index);
void lk_list_set_number_object(lk_list_t *self, lk_number_t *index, lk_object_t *value);
void lk_list_set_number_number_list(lk_list_t *self, lk_number_t *from, lk_number_t *to, lk_list_t *list);

/* info */
lk_object_t *lk_list_at_number(lk_list_t *self, lk_number_t *index);
void lk_list_flatten(lk_object_t *self, lk_scope_t *local);
#endif
