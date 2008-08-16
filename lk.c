#include "vm.h"
#include "ext.h"
#include "gc.h"
#include "map.h"
#include "parser.h"
#include "string.h"

/* eval */
int main(int argc, const char **argv) {
    lk_Vm_t *vm = lk_Vm_new();
    lk_List_t *args = lk_List_newfromargv(vm, argc, argv);
    const char *i = MF_LIBDIR "/lk.lk";
    Sequence_insert(LIST(args), 1, lk_String_newfromcstr(vm, i));
    lk_Library_set(LK_OBJ(vm->global), "ARGUMENTS", LK_OBJ(args));
    lk_Gc_resume(vm->gc);
    lk_Vm_evalfile(vm, i, "");
    lk_Vm_free(vm);
    memory_freerecycled();
    return EXIT_SUCCESS;
}
