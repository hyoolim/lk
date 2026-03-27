#include "lib.h"
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

// type
void lk_dir_type_init(lk_vm_t *vm) {
    vm->t_dir = lk_obj_alloc_type(vm->t_obj, sizeof(lk_obj_t));
}

// new
lk_obj_t *lk_dir_new_with_path(lk_vm_t *vm, lk_str_t *path) {
    lk_obj_t *self = lk_obj_alloc(vm->t_dir);
    lk_dir_init(self, path);
    return self;
}

void lk_dir_init(lk_obj_t *self, lk_str_t *path) {
    lk_str_t *dirpath = path;
    lk_str_t *name;
    int at = 0, nextat;

    if (vec_str_get(VEC(path), 0) != '/') {
        char buf[1000];

        if (getcwd(buf, 1000) != NULL) {
            lk_str_t *abs = lk_str_new_from_cstr(VM, buf);
            vec_concat(VEC(abs), VEC(VM->str_filesep));
            vec_concat(VEC(abs), VEC(path));
            dirpath = abs;
        }
    }

    while ((nextat = vec_str_find(VEC(dirpath), '/', at)) > -1) {
        at = nextat + 1;
    }

    name = lk_str_new_from_darray(VM, VEC(dirpath));
    vec_offset(VEC(name), at);

    lk_obj_set_slot_by_cstr(self, "path", NULL, LK_OBJ(dirpath));
    lk_obj_set_slot_by_cstr(self, "name", NULL, LK_OBJ(name));
}

// update
void lk_dir_create(lk_obj_t *self) {
    lk_str_t *path = LK_STRING(lk_obj_get_value_by_cstr(self, "path"));
    mkdir(CSTRING(path), S_IRWXU | S_IRWXG | S_IRWXO);
}

void lk_dir_work(lk_obj_t *self) {
    lk_str_t *path = LK_STRING(lk_obj_get_value_by_cstr(self, "path"));

    if (chdir(CSTRING(path)) != 0) {
        lk_vm_raise_errno(VM);
    }
}

// info
lk_list_t *lk_dir_items(lk_obj_t *self) {
    lk_str_t *path = LK_STRING(lk_obj_get_value_by_cstr(self, "path"));
    lk_list_t *items = lk_list_new(VM);
    DIR *dd = opendir(CSTRING(path));
    struct dirent *entry;
    struct stat info;

    while (dd != NULL) {
        errno = 0;

        if ((entry = readdir(dd)) == NULL) {
            if (errno != 0) {
                lk_vm_raise_errno(VM);
            }
            break;

        } else if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            lk_str_t *itempath = lk_str_new_from_cstr(VM, entry->d_name);
            vec_set_range(VEC(itempath), 0, 0, VEC(VM->str_filesep));
            vec_set_range(VEC(itempath), 0, 0, VEC(path));

            if (stat(CSTRING(itempath), &info) == 0 && S_ISDIR(info.st_mode)) {
                vec_ptr_push(VEC(items), lk_dir_new_with_path(VM, itempath));

            } else {
                vec_ptr_push(VEC(items), lk_file_new_with_path(VM, itempath));
            }
        }
    }
    return items;
}

// bind all c funcs to lk equiv
void lk_dir_lib_init(lk_vm_t *vm) {
    lk_obj_t *dir = vm->t_dir, *str = vm->t_str;

    lk_global_set("Directory", dir);

    // new
    lk_obj_set_cfunc_cvoid(dir, "init!", lk_dir_init, str, NULL);

    // update
    lk_obj_set_cfunc_cvoid(dir, "create!", lk_dir_create, NULL);
    lk_obj_set_cfunc_cvoid(dir, "work!", lk_dir_work, NULL);

    // info
    lk_obj_set_cfunc_creturn(dir, "items", lk_dir_items, NULL);
}
