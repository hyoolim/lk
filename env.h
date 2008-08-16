#ifndef LK_ENV_H
#define LK_ENV_H

/* type */
typedef struct lk_Env lk_Env_t;
#include "vm.h"
struct lk_Env {
    struct lk_Common obj;
};
#define LK_ENV(v) ((lk_Env_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Env_extinit);
#endif
