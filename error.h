#ifndef LK_ERROR_H
#define LK_ERROR_H

/* type */
typedef struct lk_error lk_error_t;
#define LK_ERROR(v) ((lk_error_t *)(v))
#include "vm.h"
#include "instr.h"
#include "file.h"
#include "string.h"
struct lk_error {
    struct lk_common  obj;
    lk_instr_t       *instr;
    lk_string_t      *text;
};

/* ext map */
LK_EXT_DEFINIT(lk_error_extinittypes);
LK_EXT_DEFINIT(lk_error_extinitfuncs);

/* new */
lk_error_t *lk_error_new(lk_vm_t *vm, lk_object_t *parent, const char *text);
lk_error_t *lk_error_newc(lk_vm_t *vm);
#endif
