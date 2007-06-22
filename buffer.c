#include "buffer.h"
#include "ext.h"
#include "fixnum.h"

/* ext map - types */
LK_EXT_DEFINIT(lk_buffer_extinittypes) {
    vm->t_buffer = lk_object_alloc(vm->t_glist);
    list_fin(LIST(vm->t_buffer));
    list_init(LIST(vm->t_buffer), sizeof(unsigned char), 16);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(at__buf_fi) {
    unsigned char *byte = list_get(LIST(self), INT(ARG(0)));
    RETURN(byte != NULL ? LK_O(lk_fi_new(VM, *byte)) : N);
}
static LK_EXT_DEFCFUNC(insertB__buf_fi_fi) {
    unsigned char byte = INT(ARG(1));
    list_insert(LIST(self), INT(ARG(0)), &byte);
    RETURN(self);
}
static LK_EXT_DEFCFUNC(removeB__buf_fi) {
    unsigned char *byte = list_get(LIST(self), INT(ARG(0)));
    list_remove(LIST(self), INT(ARG(0)));
    RETURN(byte != NULL ? LK_O(lk_fi_new(VM, *byte)) : N);
}
static LK_EXT_DEFCFUNC(setB__buf_fi_fi) {
    unsigned char byte = INT(ARG(1));
    list_set(LIST(self), INT(ARG(0)), &byte);
    RETURN(self);
}
LK_EXT_DEFINIT(lk_buffer_extinitfuncs) {
    lk_object_t *buf = vm->t_buffer, *fi = vm->t_fi;
    lk_ext_global("Buffer", buf);
    lk_ext_cfunc(buf, "at", at__buf_fi, fi, NULL);
    lk_ext_cfunc(buf, "insert!", insertB__buf_fi_fi, fi, fi, NULL);
    lk_ext_cfunc(buf, "remove!", removeB__buf_fi, fi, NULL);
    lk_ext_cfunc(buf, "set!", setB__buf_fi_fi, fi, fi, NULL);
}

/* new */
lk_buffer_t *lk_buffer_newfromlist(lk_vm_t *vm, list_t *buf) {
    lk_buffer_t *self = LK_BUFFER(lk_object_alloc(vm->t_buffer));
    list_copy(LIST(self), buf);
    return self;
}
