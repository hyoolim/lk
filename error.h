#ifndef LK_ERROR_H
#define LK_ERROR_H

/* type */
typedef struct lk_Error lk_Error_t;
#include "vm.h"
#include "instr.h"
#include "file.h"
#include "string.h"
struct lk_Error {
    struct lk_Common  obj;
    lk_Instr_t       *instr;
    lk_String_t      *text;
};
#define LK_ERROR(v) ((lk_Error_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Error_extinittypes);
LK_EXT_DEFINIT(lk_Error_extinitfuncs);

/* new */
lk_Error_t *lk_Error_new(lk_Vm_t *vm, lk_Object_t *parent, const char *text);
lk_Error_t *lk_Error_newc(lk_Vm_t *vm);
#endif
