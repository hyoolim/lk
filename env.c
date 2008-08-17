#include "env.h"
#include "ext.h"
#include "fixnum.h"
#include "string.h"
#include <unistd.h>

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(at__env_str) {
    const char *k = darray_toCString(LIST(ARG(0)));
    const char *v = getenv(k);
    RETURN(v != NULL ? LK_OBJ(lk_string_newfromcstr(VM, v)) : N);
}
extern char** environ;
LK_LIBRARY_DEFINECFUNCTION(size__env) {
    int c;
    for(c = 0; environ[c] != NULL; c ++) { }
    RETURN(lk_fi_new(VM, c));
}
LK_LIBRARY_DEFINECFUNCTION(keys__env) {
    lk_list_t *keys = lk_list_new(VM);
    int i, j;
    const char *v;
    for(i = 0; (v = environ[i]) != NULL; i ++) {
        for(j = 0; v[j] != '\0' && v[j] != '='; j ++) { }
        darray_pushptr(LIST(keys), lk_string_newfromdata(VM, v, j));
    }
    RETURN(keys);
}
LK_EXT_DEFINIT(lk_env_extinit) {
    lk_object_t *obj = vm->t_obj, *str = vm->t_string;
    lk_object_t *env = lk_object_allocwithsize(obj, sizeof(lk_env_t));
    lk_library_setGlobal("Environment", env);
    lk_library_setCFunction(env, "at", at__env_str, str, NULL);
    lk_library_setCFunction(env, "size", size__env, NULL);
    lk_library_setCFunction(env, "keys", keys__env, NULL);
}
