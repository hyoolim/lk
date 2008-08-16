#include "env.h"
#include "ext.h"
#include "fixnum.h"
#include "string.h"
#include <unistd.h>

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(at__env_str) {
    const char *k = Sequence_tocstr(LIST(ARG(0)));
    const char *v = getenv(k);
    RETURN(v != NULL ? LK_OBJ(lk_String_newfromcstr(VM, v)) : N);
}
extern char** environ;
LK_LIBRARY_DEFINECFUNCTION(count__env) {
    int c;
    for(c = 0; environ[c] != NULL; c ++) { }
    RETURN(lk_Fi_new(VM, c));
}
LK_LIBRARY_DEFINECFUNCTION(keys__env) {
    lk_List_t *keys = lk_List_new(VM);
    int i, j;
    const char *v;
    for(i = 0; (v = environ[i]) != NULL; i ++) {
        for(j = 0; v[j] != '\0' && v[j] != '='; j ++) { }
        Sequence_pushptr(LIST(keys), lk_String_newfromdata(VM, v, j));
    }
    RETURN(keys);
}
LK_EXT_DEFINIT(lk_Env_extinit) {
    lk_Object_t *obj = vm->t_obj, *str = vm->t_string;
    lk_Object_t *env = lk_Object_allocwithsize(obj, sizeof(lk_Env_t));
    lk_Library_setGlobal("Environment", env);
    lk_Library_setCFunction(env, "at", at__env_str, str, NULL);
    lk_Library_setCFunction(env, "count", count__env, NULL);
    lk_Library_setCFunction(env, "keys", keys__env, NULL);
}
