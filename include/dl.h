#ifndef LK_DL_H
#define LK_DL_H
#include "types.h"

/* type */
struct lk_dl {
    struct lk_common  o;
    void             *dl;
};

/* init */
void lk_dl_typeinit(lk_vm_t *vm);
void lk_dl_libinit(lk_vm_t *vm);

/* new */
void lk_dl_init_withpath_andfunc(lk_dl_t *self, lk_str_t *path, lk_str_t *funcname);

/* update */
void lk_object_set(lk_obj_t *parent, const char *k, lk_obj_t *v);
void lk_global_set(const char *k, lk_obj_t *v);
void lk_obj_set_cfield(lk_obj_t *self, const char *k, lk_obj_t *t, size_t offset);
#endif
