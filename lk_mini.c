#include "vm.h"
#include "ext.h"
#include "gc.h"
#include "map.h"
#include "parser.h"
#include "string.h"

/* eval */
int main(int argc, const char **argv) {
    lk_Vm_t *vm = lk_Vm_new();
    lk_Library_set(LK_OBJ(vm->global), "ARGUMENTS",
    LK_OBJ(lk_List_newfromargv(vm, argc, argv)));
    lk_Gc_resume(vm->gc);
    lk_Vm_evalfile(vm, argv[1], "");
    lk_Vm_free(vm);
    memory_freerecycled();
    return EXIT_SUCCESS;
}
