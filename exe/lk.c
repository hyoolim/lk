#include "lib.h"

/* eval */
int main(int argc, const char **argv) {
    lk_vm_t *vm = lk_vm_new();
    lk_list_t *args = lk_list_newfromargv(vm, argc, argv);
    const char *i = PREFIX "/lib/lk/lk.lk";
    darray_insert(DARRAY(args), 1, lk_str_new_fromcstr(vm, i));
    lk_object_set(LK_OBJ(vm->global), "ARGUMENTS", LK_OBJ(args));
    lk_gc_resume(vm->gc);
    lk_vm_evalfile(vm, i, "");
    lk_vm_free(vm);
    mem_freerecycled();
    return EXIT_SUCCESS;
}
