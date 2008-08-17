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
LK_EXT_DEFINIT(lk_Folder_extinittypes) {
    vm->t_folder = lk_Object_allocwithsize(vm->t_obj, sizeof(lk_Folder_t));
    lk_Object_setmarkfunc(vm->t_folder, mark__Folder);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(init__Folder_str) {
    LK_FOLDER(self)->path = LK_STRING(ARG(0));
}
LK_LIBRARY_DEFINECFUNCTION(items__Folder) {
    lk_List_t *items = lk_List_new(VM);
    lk_String_t *fullPath = lk_String_new(VM);
    DIR *dir = opendir(CSTR(LK_FOLDER(self)->path));
    struct dirent *dirEntry;
    struct stat fileInfo;
    while(dir != NULL) {
        errno = 0;
        if((dirEntry = readdir(dir)) == NULL) break;
        else {
            Sequence_t *filename = string_allocfromcstr(dirEntry->d_name);
            Sequence_clear(LIST(fullPath));
            Sequence_concat(LIST(fullPath), LIST(LK_FOLDER(self)->path));
            Sequence_concat(LIST(fullPath), LIST(VM->str_filesep));
            Sequence_concat(LIST(fullPath), filename);
            Sequence_pushptr(LIST(items), fullPath);
            fullPath = lk_String_new(VM);
            Sequence_free(filename);
        }
    }
    if(errno == 0) {
        RETURN(items);
    } else {
        lk_Vm_raiseerrno(VM);
    }
}
LK_EXT_DEFINIT(lk_Folder_extinitfuncs) {
    lk_Object_t *folder = vm->t_folder, *str = vm->t_string;
    lk_Library_setGlobal("Folder", folder);
    lk_Library_setCFunction(folder, "init", init__Folder_str, str, NULL);
    lk_Library_setCFunction(folder, "items", items__Folder, NULL);
}
