#ifndef LK_ENV_H
#define LK_ENV_H

/* type */
typedef struct lk_env lk_env_t;
#include "vm.h"
struct lk_env {
    struct lk_common co;
};
#define LK_ENV(v) ((lk_env_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_env_extinit);
#endif
