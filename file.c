#include "lib.h"
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

/* type */
static LK_OBJ_DEFMARKFUNC(mark_file) {
    mark(LK_OBJ(LK_FILE(self)->path));
}
static void free_file(lk_obj_t *self) {
    lk_file_close(LK_FILE(self));
}
void lk_file_typeinit(lk_vm_t *vm) {
    vm->t_file = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_file_t));
    lk_obj_setmarkfunc(vm->t_file, mark_file);
    lk_obj_setfreefunc(vm->t_file, free_file);
    LK_FILE(vm->t_stdin = lk_obj_alloc(vm->t_file))->fd = stdin;
    LK_FILE(vm->t_stdout = lk_obj_alloc(vm->t_file))->fd = stdout;
    LK_FILE(vm->t_stderr = lk_obj_alloc(vm->t_file))->fd = stderr;
}

/* new */
lk_file_t *lk_file_new_withpath(lk_vm_t *vm, lk_str_t *path) {
    lk_file_t *self = LK_FILE(lk_obj_alloc(vm->t_file));
    lk_file_init_str(self, path);
    return self;
}
void lk_file_init_str(lk_file_t *self, lk_str_t *path) {
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
void lk_file_close(lk_file_t *self) {
    if(self->fd != NULL) {
        if(fclose(self->fd) != 0) {
            lk_vm_raiseerrno(VM);
        }
        self->fd = NULL;
    }
}
void lk_file_delete(lk_file_t *self) {
    if(remove(CSTRING(self->path)) != 0) {
        lk_vm_raiseerrno(VM);
    }
}
void lk_file_flush(lk_file_t *self) {
    if(self->fd != NULL && fflush(self->fd) != 0) {
        lk_vm_raiseerrno(VM);
    }
}
void lk_file_move_str(lk_file_t *self, lk_str_t *dest) {
    if(rename(CSTRING(self->path), CSTRING(dest)) != 0) {
        lk_vm_raiseerrno(VM);
    }
}
void lk_file_open(lk_file_t *self, lk_str_t *mode) {
    if(self->fd != NULL) {
        BUG("ReadableFile->st.file should be NULL");
    }
    self->fd = fopen(CSTRING(self->path), CSTRING(mode));
    if(self->fd == NULL) {
        lk_vm_raiseerrno(VM);
    }
}
void lk_file_write_str(lk_file_t *self, lk_str_t *text) {
    if(self->fd == NULL) {
        BUG("WritableFile->st.file should NEVER be NULL");
    }
    darray_print_tostream(DARRAY(text), self->fd);
}

/* info */
lk_bool_t *lk_file_isdirectory(lk_file_t *self) {
    struct stat info;
    return stat(CSTRING(self->path), &info) == 0 && S_ISDIR(info.st_mode) ? TRUE : FALSE;
}
lk_bool_t *lk_file_isexecutable(lk_file_t *self) {
    return access(CSTRING(self->path), X_OK) == 0 ? TRUE : FALSE;
}
lk_bool_t *lk_file_isexists(lk_file_t *self) {
    return access(CSTRING(self->path), F_OK) == 0 ? TRUE : FALSE;
}
lk_str_t *lk_file_read_num(lk_file_t *self, lk_num_t *length) {
    if(self->fd == NULL) {
        BUG("ReadableFile->st.file should NEVER be NULL");
    } else {
        darray_t *c = darray_allocfromfile(self->fd, CNUMBER(length));
        return c != NULL ? lk_str_new_fromdarray(VM, c) : LK_STRING(NIL);
    }
}
lk_str_t *lk_file_readall(lk_file_t *self) {
    if(self->fd == NULL) {
        BUG("ReadableFile->st.file should NEVER be NULL");
    } else {
        darray_t *c = str_allocfromfile(self->fd);
        return c != NULL ? lk_str_new_fromdarray(VM, c) : LK_STRING(NIL);
    }
}
lk_str_t *lk_file_readuntil_char(lk_file_t *self, lk_char_t *until) {
    if(self->fd == NULL) {
        BUG("ReadableFile->st.file should NEVER be NULL");
    } else {
        darray_t *c = darray_alloc_fromfile_untilchar(self->fd, CHAR(until));
        return c != NULL ? lk_str_new_fromdarray(VM, c) : LK_STRING(NIL);
    }
}
lk_str_t *lk_file_readuntil_charset(lk_file_t *self, lk_charset_t *until) {
    if(self->fd == NULL) {
        BUG("ReadableFile->st.file should NEVER be NULL");
    } else {
        darray_t *c = darray_alloc_fromfile_untilcharset(self->fd, CHARSET(until));
        return c != NULL ? lk_str_new_fromdarray(VM, c) : LK_STRING(NIL);
    }
}
lk_bool_t *lk_file_isreadable(lk_file_t *self) {
    return access(CSTRING(self->path), R_OK) == 0 ? TRUE : FALSE;
}
lk_num_t *lk_file_size(lk_file_t *self) {
    struct stat info;
    if(stat(CSTRING(self->path), &info) != 0) {
        lk_vm_raiseerrno(VM);
    }
    return lk_num_new(VM, info.st_size);
}
lk_bool_t *lk_file_iswritable(lk_file_t *self) {
    return access(CSTRING(self->path), W_OK) == 0 ? TRUE : FALSE;
}

/* bind all c funcs to lk equiv */
void lk_file_libinit(lk_vm_t *vm) {
    lk_obj_t *file = vm->t_file, *str = vm->t_str, *num = vm->t_num, *ch = vm->t_char, *charset = vm->t_charset;
    lk_global_set("File", file);

    /* props */
    lk_obj_set_cfield(file, "path", str, offsetof(lk_file_t, path));
    lk_obj_set_cfield(file, "name", str, offsetof(lk_file_t, name));

    /* new */
    lk_obj_set_cfunc_cvoid(file, "init!", lk_file_init_str, str, NULL);

    /* update */
    lk_obj_set_cfunc_cvoid(file, "close!", lk_file_close, NULL);
    lk_obj_set_cfunc_cvoid(file, "delete!", lk_file_delete, NULL);
    lk_obj_set_cfunc_cvoid(file, "flush!", lk_file_flush, NULL);
    lk_obj_set_cfunc_cvoid(file, "move!", lk_file_move_str, str, NULL);
    lk_obj_set_cfunc_cvoid(file, "open", lk_file_open, str, NULL);
    lk_obj_set_cfunc_cvoid(file, "write", lk_file_write_str, str, NULL);

    /* info */
    lk_obj_set_cfunc_creturn(file, "directory?", lk_file_isdirectory, NULL);
    lk_obj_set_cfunc_creturn(file, "executable?", lk_file_isexecutable, NULL);
    lk_obj_set_cfunc_creturn(file, "exists?", lk_file_isexists, NULL);
    lk_obj_set_cfunc_creturn(file, "read", lk_file_read_num, num, NULL);
    lk_obj_set_cfunc_creturn(file, "readAll", lk_file_readall, NULL);
    lk_obj_set_cfunc_creturn(file, "readUntil", lk_file_readuntil_char, ch, NULL);
    lk_obj_set_cfunc_creturn(file, "readUntil", lk_file_readuntil_charset, charset, NULL);
    lk_obj_set_cfunc_creturn(file, "readable?", lk_file_isreadable, NULL);
    lk_obj_set_cfunc_creturn(file, "size", lk_file_size, NULL);
    lk_obj_set_cfunc_creturn(file, "writable?", lk_file_iswritable, NULL);

    /* standard pipes */
    lk_global_set("STDIN", vm->t_stdin);
    lk_global_set("STDOUT", vm->t_stdout);
    lk_global_set("STDERR", vm->t_stderr);
}
