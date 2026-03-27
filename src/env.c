#include "lib.h"
#include <unistd.h>

// ext map - funcs
static void at_env_str(lk_obj_t *self, lk_scope_t *local) {
    const char *k = vec_str_tocstr(VEC(ARG(0)));
    const char *v = getenv(k);
    RETURN(v != NULL ? LK_OBJ(lk_str_new_from_cstr(VM, v)) : NIL);
}

extern char **environ;

static void size_env(lk_obj_t *self, lk_scope_t *local) {
    int c = 0;

    for (; environ[c] != NULL; c++) {
    }
    RETURN(lk_num_new(VM, c));
}

static void keys_env(lk_obj_t *self, lk_scope_t *local) {
    lk_list_t *keys = lk_list_new(VM);

    for (int i = 0; environ[i] != NULL; i++) {
        const char *v = environ[i];
        int j;

        for (j = 0; v[j] != '\0' && v[j] != '='; j++) {
        }
        vec_ptr_push(VEC(keys), lk_str_new_from_data(VM, v, j));
    }
    RETURN(keys);
}

void lk_env_ext_init(lk_vm_t *vm) {
    lk_obj_t *obj = vm->t_obj, *str = vm->t_str;
    lk_obj_t *env = lk_obj_alloc_type(obj, sizeof(lk_env_t));
    lk_global_set("Environment", env);
    lk_obj_set_cfunc_lk(env, "at", at_env_str, str, NULL);
    lk_obj_set_cfunc_lk(env, "size", size_env, NULL);
    lk_obj_set_cfunc_lk(env, "keys", keys_env, NULL);
}
