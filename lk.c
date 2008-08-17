#include "vm.h"
#include "ext.h"
#include "gc.h"
#include "map.h"
#include "parser.h"
#include "string.h"

/* eval */
int main(int argc, const char **argv) {
    lk_vm_t *vm = lk_vm_new();
    lk_list_t *args = lk_list_newfromargv(vm, argc, argv);
    const char *i = MF_LIBDIR "/lk.lk";
    darray_insert(DARRAY(args), 1, lk_string_newFromCString(vm, i));
    lk_lib_setObject(LK_OBJ(vm->global), "ARGUMENTS", LK_OBJ(args));
    lk_gc_resume(vm->gc);
    lk_vm_evalfile(vm, i, "");
    lk_vm_free(vm);
    memory_freerecycled();
    return EXIT_SUCCESS;
}
