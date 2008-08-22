#ifndef LK_ERROR_H
#define LK_ERROR_H

/* type */
typedef struct lk_error lk_error_t;
#define LK_ERROR(v) ((lk_error_t *)(v))
#include "vm.h"
#include "instr.h"
#include "string.h"
struct lk_error {
    struct lk_common  o;
    lk_instr_t       *instr;
    lk_string_t      *message;
};

/* init */
LK_LIB_DEFINEINIT(lk_error_libPreInit);
LK_LIB_DEFINEINIT(lk_error_libInit);

/* new */
lk_error_t *lk_error_new(lk_vm_t *vm, lk_object_t *parent, const char *message);
#endif
