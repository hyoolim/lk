#include "vm.h"
#include "ext.h"
#include "gc.h"
#include "parser.h"
#include "string.h"
#include "dict.h"

/* eval */
int main(int argc, const char **argv) {
    lk_vm_t *vm = lk_vm_new();
    lk_list_t *args = lk_list_newfromargv(vm, argc, argv);
    const char *i = MF_LIBDIR "/lk.lk";
    pt_list_insert(LIST(args), 1, lk_string_newfromcstr(vm, i));
    lk_ext_set(LK_O(vm->global), "ARGUMENTS", LK_O(args));
    lk_gc_resume(vm->gc);
    lk_vm_evalfile(vm, i, "");
    lk_vm_free(vm);
    return EXIT_SUCCESS;
}
