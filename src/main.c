#include "lib.h"

int main(int argc, const char **argv) {
    int i = 1;
    lk_vm_t *vm = lk_vm_new();
    lk_list_t *libpaths = lk_list_new(vm);
    lk_list_t *args = lk_list_new(vm);
    lk_str_t *script, *initfile = lk_str_new_from_cstr(vm, "Init.lk");
    lk_obj_t *tmp;
    lk_str_t *tmp_path;

    // options for vm
    lk_object_set(LK_OBJ(vm->t_dl), "paths", LK_OBJ(libpaths));
    vec_ptr_insert(VEC(libpaths), 0, lk_str_new_from_cstr(vm, LK_INSTALL_PATH "/lib/lk"));

    for (; i < argc; i++) {
        if (argv[i][0] != '-') {
            break;

        } else {
            switch (argv[i][1]) {
            case 'l':
                if (++i < argc) {
                    vec_ptr_insert(VEC(libpaths), 0, lk_str_new_from_cstr(vm, argv[i]));

                } else {
                    fprintf(stderr, "%s: -l requires an argument\n", argv[0]);
                    lk_vm_free(vm);
                    mem_free_recycled();
                    return EXIT_FAILURE;
                }
                break;
            case '-':
                break;
            default:
                fprintf(stderr, "%s: -%c is not a valid option\n", argv[0], argv[i][1]);
                lk_vm_free(vm);
                mem_free_recycled();
                return EXIT_FAILURE;
            }
        }
    }

    // get script name
    if (i >= argc) {
        fprintf(stderr, "%s: script file is required\n", argv[0]);
        lk_vm_free(vm);
        mem_free_recycled();
        return EXIT_FAILURE;
    }
    script = lk_str_new_from_cstr(vm, argv[i++]);

    // pass rest of the arguments to lk program
    lk_global_set("arguments", LK_OBJ(args));
    lk_global_set("args", LK_OBJ(args));

    for (; i < argc; i++) {
        vec_ptr_push(VEC(args), lk_str_new_from_cstr(vm, argv[i]));
    }

    // load the initial lib
    VEC_EACH_PTR(
        VEC(libpaths), i, path, tmp = lk_file_new_with_path(vm, LK_STRING(lk_obj_clone(LK_OBJ(path))));
        tmp_path = LK_STRING(lk_obj_get_value_by_cstr(tmp, "path"));
        vec_concat(VEC(tmp_path), VEC(vm->str_filesep));
        vec_concat(VEC(tmp_path), VEC(initfile));
        if (lk_file_is_exists(tmp) == vm->t_true) {
            lk_vm_eval_file(vm, vec_str_tocstr(VEC(tmp_path)), "");
            break;
        });

    // run the script
    lk_gc_resume(vm->gc);
    lk_vm_eval_file(vm, vec_str_tocstr(VEC(script)), "");
    lk_vm_free(vm);
    mem_free_recycled();
    return EXIT_SUCCESS;
}
