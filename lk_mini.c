#include "ext.h"

/* eval */
int main(int argc, const char **argv) {
    lk_vm_t *vm = lk_vm_new();
    lk_lib_setObject(LK_OBJ(vm->global), "ARGUMENTS",
    LK_OBJ(lk_list_newfromargv(vm, argc, argv)));
    lk_gc_resume(vm->gc);
    lk_vm_evalfile(vm, argv[1], "");
    lk_vm_free(vm);
    mem_freerecycled();
    return EXIT_SUCCESS;
}
