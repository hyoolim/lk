#include "lib.h"
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

/* type */
static LK_OBJ_DEFMARKFUNC(mark_dir) {
    mark(LK_OBJ(LK_DIR(self)->path));
}
void lk_dir_typeinit(lk_vm_t *vm) {
    vm->t_dir = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_dir_t));
    lk_obj_setmarkfunc(vm->t_dir, mark_dir);
}

/* new */
lk_dir_t *lk_dir_new_withpath(lk_vm_t *vm, lk_str_t *path) {
    lk_dir_t *self = LK_DIR(lk_obj_alloc(vm->t_dir));
    lk_dir_init(self, path);
    return self;
}
void lk_dir_init(lk_dir_t *self, lk_str_t *path) {
    int at = 0, nextat;
    if(darray_getuchar(DARRAY(path), 0) == '/') {
        self->path = path;
    } else {
        char buf[1000];
        if(getcwd(buf, 1000) != NULL) {
            lk_str_t *abs = lk_str_new_fromcstr(VM, buf);
            darray_concat(DARRAY(abs), DARRAY(VM->str_filesep));
            darray_concat(DARRAY(abs), DARRAY(path));
            self->path = abs;
        }
    }
    while((nextat = darray_find_char(DARRAY(self->path), '/', at)) > -1) {
        at = nextat + 1;
    }
    self->name = lk_str_new_fromdarray(VM, DARRAY(self->path));
    darray_offset(DARRAY(self->name), at);
}

/* update */
void lk_dir_create(lk_dir_t *self) {
    mkdir(CSTRING(self->path), S_IRWXU | S_IRWXG | S_IRWXO);
}

/* info */
lk_list_t *lk_dir_items(lk_dir_t *self) {
    lk_list_t *items = lk_list_new(VM);
    DIR *dd = opendir(CSTRING(self->path));
    struct dirent *entry;
    struct stat info;
    while(dd != NULL) {
        errno = 0;
        if((entry = readdir(dd)) == NULL) {
            if(errno != 0) {
                lk_vm_raiseerrno(VM);
            }
            break;
        } else if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            lk_str_t *path = lk_str_new_fromcstr(VM, entry->d_name);
            darray_setrange(DARRAY(path), 0, 0, DARRAY(VM->str_filesep));
            darray_setrange(DARRAY(path), 0, 0, DARRAY(self->path));
            if(stat(CSTRING(path), &info) == 0 && S_ISDIR(info.st_mode)) {
                darray_pushptr(DARRAY(items), lk_dir_new_withpath(VM, path));
            } else {
                darray_pushptr(DARRAY(items), lk_file_new_withpath(VM, path));
            }
        }
    }
    return items;
}

/* bind all c funcs to lk equiv */
void lk_dir_libinit(lk_vm_t *vm) {
    lk_obj_t *dir = vm->t_dir, *str = vm->t_str;
    lk_global_set("Directory", dir);

    /* props */
    lk_obj_set_cfield(dir, "name", str, offsetof(lk_dir_t, name));
    lk_obj_set_cfield(dir, "path", str, offsetof(lk_dir_t, path));

    /* new */
    lk_obj_set_cfunc_cvoid(dir, "init!", lk_dir_init, str, NULL);

    /* update */
    lk_obj_set_cfunc_cvoid(dir, "create!", lk_dir_create, NULL);

    /* info */
    lk_obj_set_cfunc_creturn(dir, "items", lk_dir_items, NULL);
}
