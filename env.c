#include "env.h"
#include "ext.h"
#include "number.h"
#include "string.h"
#include <unistd.h>

/* ext map - funcs */
LK_LIB_DEFINECFUNC(at__env_str) {
    const char *k = darray_toCString(DARRAY(ARG(0)));
    const char *v = getenv(k);
    RETURN(v != NULL ? LK_OBJ(lk_string_newFromCString(VM, v)) : NIL);
}
extern char** environ;
LK_LIB_DEFINECFUNC(size__env) {
    int c;
    for(c = 0; environ[c] != NULL; c ++) { }
    RETURN(lk_number_new(VM, c));
}
LK_LIB_DEFINECFUNC(keys__env) {
    lk_list_t *keys = lk_list_new(VM);
    int i, j;
    const char *v;
    for(i = 0; (v = environ[i]) != NULL; i ++) {
        for(j = 0; v[j] != '\0' && v[j] != '='; j ++) { }
        darray_pushptr(DARRAY(keys), lk_string_newFromData(VM, v, j));
    }
    RETURN(keys);
}
LK_LIB_DEFINEINIT(lk_env_extinit) {
    lk_object_t *obj = vm->t_obj, *str = vm->t_string;
    lk_object_t *env = lk_object_allocwithsize(obj, sizeof(lk_env_t));
    lk_lib_setGlobal("Environment", env);
    lk_lib_setCFunc(env, "at", at__env_str, str, NULL);
    lk_lib_setCFunc(env, "size", size__env, NULL);
    lk_lib_setCFunc(env, "keys", keys__env, NULL);
}
