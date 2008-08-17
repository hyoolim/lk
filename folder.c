#include "folder.h"
#include "ext.h"
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark__Folder) {
    mark(LK_OBJ(LK_FOLDER(self)->path));
}
LK_EXT_DEFINIT(lk_folder_extinittypes) {
    vm->t_folder = lk_object_allocwithsize(vm->t_obj, sizeof(lk_folder_t));
    lk_object_setmarkfunc(vm->t_folder, mark__Folder);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(init__Folder_str) {
    LK_FOLDER(self)->path = LK_STRING(ARG(0));
}
LK_LIBRARY_DEFINECFUNCTION(items__Folder) {
    lk_list_t *items = lk_list_new(VM);
    lk_string_t *fullPath = lk_string_new(VM);
    DIR *dir = opendir(CSTR(LK_FOLDER(self)->path));
    struct dirent *dirEntry;
    struct stat fileInfo;
    while(dir != NULL) {
        errno = 0;
        if((dirEntry = readdir(dir)) == NULL) break;
        else {
            darray_t *filename = darray_allocFromCString(dirEntry->d_name);
            darray_clear(LIST(fullPath));
            darray_concat(LIST(fullPath), LIST(LK_FOLDER(self)->path));
            darray_concat(LIST(fullPath), LIST(VM->str_filesep));
            darray_concat(LIST(fullPath), filename);
            darray_pushptr(LIST(items), fullPath);
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
LK_EXT_DEFINIT(lk_folder_extinitfuncs) {
    lk_object_t *folder = vm->t_folder, *str = vm->t_string;
    lk_library_setGlobal("Folder", folder);
    lk_library_setCFunction(folder, "init", init__Folder_str, str, NULL);
    lk_library_setCFunction(folder, "items", items__Folder, NULL);
}
