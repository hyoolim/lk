#include "lib.h"
int main(int argc, const char **argv) {
    int i = 1;
    lk_vm_t *vm = lk_vm_new();
    lk_list_t *libpaths = lk_list_new(vm);
    lk_list_t *args = lk_list_new(vm);
    lk_str_t *script, *initfile = lk_str_new_fromcstr(vm, "Init.lk");
    lk_file_t *tmp;

    /* options for vm */
    lk_object_set(LK_OBJ(vm->t_dl), "paths", LK_OBJ(libpaths));
    darray_insertptr(DARRAY(libpaths), 0, lk_str_new_fromcstr(vm, PREFIX "/lib/lk"));
    for(; i < argc; i ++) {
        if(argv[i][0] != '-') {
            break;
        } else {
            switch(argv[i][1]) {
                case 'l':
                    if(++ i < argc) {
                        darray_insertptr(DARRAY(libpaths), 0, lk_str_new_fromcstr(vm, argv[i]));
                    } else {
                        fprintf(stderr, "%s: -l requires an argument\n", argv[0]);
                        lk_vm_free(vm);
                        mem_freerecycled();
                        return EXIT_FAILURE;
                    }
                    break;
                case '-':
                    break;
                default:
                    fprintf(stderr, "%s: -%c is not a valid option\n", argv[0], argv[i][1]);
                    lk_vm_free(vm);
                    mem_freerecycled();
                    return EXIT_FAILURE;
            }
        }
    }

    /* get script name */
    if(i >= argc) {
        fprintf(stderr, "%s: script file is required\n", argv[0]);
        lk_vm_free(vm);
        mem_freerecycled();
        return EXIT_FAILURE;
    }
    script = lk_str_new_fromcstr(vm, argv[i ++]);

    /* pass rest of the arguments to lk program */
    lk_global_set("arguments", LK_OBJ(args));
    lk_global_set("args", LK_OBJ(args));
    for(; i < argc; i ++) {
        darray_pushptr(DARRAY(args), lk_str_new_fromcstr(vm, argv[i]));
    }

    /* load the initial lib */
    LIST_EACHPTR(DARRAY(libpaths), i, path,
        tmp = lk_file_new_withpath(vm, LK_STRING(lk_obj_clone(LK_OBJ(path))));
        darray_concat(DARRAY(tmp->path), DARRAY(vm->str_filesep));
        darray_concat(DARRAY(tmp->path), DARRAY(initfile));
        if(lk_file_isexists(tmp) == vm->t_true) {
            lk_vm_evalfile(vm, darray_tocstr(DARRAY(tmp->path)), "");
            break;
        }
    );

    /* run the script */
    lk_gc_resume(vm->gc);
    lk_vm_evalfile(vm, darray_tocstr(DARRAY(script)), "");
    lk_vm_free(vm);
    mem_freerecycled();
    return EXIT_SUCCESS;
}
