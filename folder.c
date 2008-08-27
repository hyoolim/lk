#include "ext.h"
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

/* type */
static LK_OBJ_DEFMARKFUNC(mark_folder) {
    mark(LK_OBJ(LK_FOLDER(self)->path));
}
void lk_folder_typeinit(lk_vm_t *vm) {
    vm->t_folder = lk_obj_allocWithSize(vm->t_obj, sizeof(lk_folder_t));
    lk_obj_setmarkfunc(vm->t_folder, mark_folder);
}

/* new */
void lk_folder_init(lk_obj_t *self, lk_str_t *path) {
    LK_FOLDER(self)->path = LK_STRING(path);
}

/* info */
lk_list_t *lk_folder_items(lk_obj_t *self) {
    lk_list_t *items = lk_list_new(VM);
    lk_str_t *fullPath = lk_str_new(VM);
    DIR *dir = opendir(CSTRING(LK_FOLDER(self)->path));
    struct dirent *dirEntry;
    while(dir != NULL) {
        errno = 0;
        if((dirEntry = readdir(dir)) == NULL) break;
        else {
            darray_t *filename = darray_allocFromCString(dirEntry->d_name);
            darray_clear(DARRAY(fullPath));
            darray_concat(DARRAY(fullPath), DARRAY(LK_FOLDER(self)->path));
            darray_concat(DARRAY(fullPath), DARRAY(VM->str_filesep));
            darray_concat(DARRAY(fullPath), filename);
            darray_pushptr(DARRAY(items), fullPath);
            fullPath = lk_str_new(VM);
            darray_free(filename);
        }
    }
    if(errno == 0) {
        return items;
    } else {
        lk_vm_raiseerrno(VM);
    }
}

/* bind all c funcs to lk equiv */
void lk_folder_libinit(lk_vm_t *vm) {
    lk_obj_t *folder = vm->t_folder, *str = vm->t_str;
    lk_lib_setGlobal("Folder", folder);

    /* new */
    lk_obj_set_cfunc_cvoid(folder, "init!", lk_folder_init, str, NULL);

    /* info */
    lk_obj_set_cfunc_creturn(folder, "items", lk_folder_items, NULL);
}
