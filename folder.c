#include "ext.h"
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark_Folder) {
    mark(LK_OBJ(LK_FOLDER(self)->path));
}
void lk_folder_typeinit(lk_vm_t *vm) {
    vm->t_folder = lk_object_allocWithSize(vm->t_object, sizeof(lk_folder_t));
    lk_object_setmarkfunc(vm->t_folder, mark_Folder);
}

/* ext map - funcs */
static void init_Folder_str(lk_object_t *self, lk_scope_t *local) {
    LK_FOLDER(self)->path = LK_STRING(ARG(0));
}
static void items_Folder(lk_object_t *self, lk_scope_t *local) {
    lk_list_t *items = lk_list_new(VM);
    lk_string_t *fullPath = lk_string_new(VM);
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
            fullPath = lk_string_new(VM);
            darray_free(filename);
        }
    }
    if(errno == 0) {
        RETURN(items);
    } else {
        lk_vm_raiseerrno(VM);
    }
}
void lk_folder_libinit(lk_vm_t *vm) {
    lk_object_t *folder = vm->t_folder, *str = vm->t_string;
    lk_lib_setGlobal("Folder", folder);
    lk_object_set_cfunc_lk(folder, "init!", init_Folder_str, str, NULL);
    lk_object_set_cfunc_lk(folder, "items", items_Folder, NULL);
}
