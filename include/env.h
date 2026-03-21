#ifndef LK_ENV_H
#define LK_ENV_H
#include "types.h"

// type
struct lk_env {
    struct lk_common o;
};

// init
void lk_env_ext_init(lk_vm_t *vm);
#endif
