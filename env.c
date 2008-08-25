#include "ext.h"
#include <unistd.h>

/* ext map - funcs */
static void at_env_str(lk_object_t *self, lk_scope_t *local) {
    const char *k = darray_toCString(DARRAY(ARG(0)));
    const char *v = getenv(k);
    RETURN(v != NULL ? LK_OBJ(lk_string_newFromCString(VM, v)) : NIL);
}
extern char** environ;
static void size_env(lk_object_t *self, lk_scope_t *local) {
    int c;
    for(c = 0; environ[c] != NULL; c ++) { }
    RETURN(lk_number_new(VM, c));
}
static void keys_env(lk_object_t *self, lk_scope_t *local) {
    lk_list_t *keys = lk_list_new(VM);
    int i, j;
    const char *v;
    for(i = 0; (v = environ[i]) != NULL; i ++) {
        for(j = 0; v[j] != '\0' && v[j] != '='; j ++) { }
        darray_pushptr(DARRAY(keys), lk_string_newFromData(VM, v, j));
    }
    RETURN(keys);
}
void lk_env_extinit(lk_vm_t *vm) {
    lk_object_t *obj = vm->t_object, *str = vm->t_string;
    lk_object_t *env = lk_object_allocWithSize(obj, sizeof(lk_env_t));
    lk_lib_setGlobal("Environment", env);
    lk_object_set_cfunc_lk(env, "at", at_env_str, str, NULL);
    lk_object_set_cfunc_lk(env, "size", size_env, NULL);
    lk_object_set_cfunc_lk(env, "keys", keys_env, NULL);
}
