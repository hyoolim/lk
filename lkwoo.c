#include "vm.h"
#include "ext.h"
#include "gc.h"
#include "parser.h"
#include "string.h"
#include "dict.h"

/* placeholder optional init */ 
LK_EXT_DEFINIT(lk_optional_extinit) { 
}

/* eval */
int main(int argc, const char **argv) {
    lk_vm_t *vm = lk_vm_new();
    lk_ext_set(LK_O(vm->global), "ARGUMENTS",
    LK_O(lk_list_newfromargv(vm, argc, argv)));
    lk_gc_resume(vm->gc);
    lk_vm_evalfile(vm, argv[1], "");
    lk_vm_free(vm);
    return EXIT_SUCCESS;
}
