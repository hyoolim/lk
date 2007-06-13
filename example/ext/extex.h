#ifndef LK_EXTEX_H
#define LK_EXTEX_H
#include <lk/vm.h>
#include <lk/fixnum.h>

/* type */
typedef struct lk_extex_t {
    struct lk_common_t c;
    lk_fixint_t *x;
    lk_fixint_t *y;
} lk_extex_t;

/* cast */
#define LK_EXTEX(o) ((lk_extex_t *)(o))

/* ext map */
LK_DEFGLOBAL_PROTO(ExtensionExample);
LK_DEFEXTINIT_PROTO(lk_extex_extinit);
#endif
