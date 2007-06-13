#include "env.h"
#include "ext.h"
#include "fixnum.h"
#include "string.h"
#include <unistd.h>

/* ext map - funcs */
static LK_EXT_DEFCFUNC(at__env_str) {
    const char *k = pt_list_tocstr(LIST(ARG(0)));
    const char *v = getenv(k);
    RETURN(v != NULL ? LK_O(lk_string_newfromcstr(VM, v)) : N);
}
extern char** environ;
static LK_EXT_DEFCFUNC(count__env) {
    int c;
    for(c = 0; environ[c] != NULL; c ++) { }
    RETURN(lk_fi_new(VM, c));
}
static LK_EXT_DEFCFUNC(keys__env) {
    lk_list_t *keys = lk_list_new(VM);
    int i, j;
    const char *v;
    for(i = 0; (v = environ[i]) != NULL; i ++) {
        for(j = 0; v[j] != '\0' && v[j] != '='; j ++) { }
        pt_list_pushptr(LIST(keys), lk_string_newfromdata(VM, v, j));
    }
    RETURN(keys);
}
LK_EXT_DEFINIT(lk_env_extinit) {
    lk_object_t *obj = vm->t_object, *str = vm->t_string;
    lk_object_t *env = lk_object_allocwithsize(obj, sizeof(lk_env_t));
    lk_ext_global("Environment", env);
    lk_ext_cfunc(env, "at", at__env_str, str, NULL);
    lk_ext_cfunc(env, "count", count__env, NULL);
    lk_ext_cfunc(env, "keys", keys__env, NULL);
}
