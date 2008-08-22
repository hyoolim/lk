#ifndef LK_ENV_H
#define LK_ENV_H

/* type */
typedef struct lk_env lk_env_t;
#define LK_ENV(object) ((lk_env_t *)(object))
#include "vm.h"
struct lk_env {
    struct lk_common o;
};

/* init */
void lk_env_extinit(lk_vm_t *vm);
#endif
