#include "lib.h"
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

// type
static LK_OBJ_DEFMARKFUNC(mark_file) {
    mark(LK_OBJ(LK_FILE(self)->path));
}

static void free_file(lk_obj_t *self) {
    lk_file_close(LK_FILE(self));
}

void lk_file_type_init(lk_vm_t *vm) {
    vm->t_file = lk_obj_alloc_type(vm->t_obj, sizeof(lk_file_t));
    lk_obj_set_mark_func(vm->t_file, mark_file);
    lk_obj_set_free_func(vm->t_file, free_file);
    LK_FILE(vm->t_stdin = lk_obj_alloc(vm->t_file))->fd = stdin;
    LK_FILE(vm->t_stdout = lk_obj_alloc(vm->t_file))->fd = stdout;
    LK_FILE(vm->t_stderr = lk_obj_alloc(vm->t_file))->fd = stderr;
}

// new
lk_file_t *lk_file_new_with_path(lk_vm_t *vm, lk_str_t *path) {
    lk_file_t *self = LK_FILE(lk_obj_alloc(vm->t_file));
    lk_file_init_str(self, path);
    return self;
}

void lk_file_init_str(lk_file_t *self, lk_str_t *path) {
    int at = 0, nextat;

    if (vec_str_get(VEC(path), 0) == '/') {
        self->path = path;

    } else {
        char buf[1000];

        if (getcwd(buf, 1000) != NULL) {
            lk_str_t *abs = lk_str_new_from_cstr(VM, buf);
            vec_concat(VEC(abs), VEC(VM->str_filesep));
            vec_concat(VEC(abs), VEC(path));
            self->path = abs;
        }
    }

    while ((nextat = vec_str_find(VEC(self->path), '/', at)) > -1) {
        at = nextat + 1;
    }
    self->name = lk_str_new_from_darray(VM, VEC(self->path));
    vec_offset(VEC(self->name), at);
}

// update
void lk_file_close(lk_file_t *self) {
    if (self->fd != NULL) {
        if (fclose(self->fd) != 0) {
            lk_vm_raise_errno(VM);
        }

        self->fd = NULL;
    }
}

void lk_file_delete(lk_file_t *self) {
    if (remove(CSTRING(self->path)) != 0) {
        lk_vm_raise_errno(VM);
    }
}

void lk_file_flush(lk_file_t *self) {
    if (self->fd != NULL && fflush(self->fd) != 0) {
        lk_vm_raise_errno(VM);
    }
}

void lk_file_move_str(lk_file_t *self, lk_str_t *dest) {
    if (rename(CSTRING(self->path), CSTRING(dest)) != 0) {
        lk_vm_raise_errno(VM);
    }
}

void lk_file_open(lk_file_t *self, lk_str_t *mode) {
    if (self->fd != NULL) {
        BUG("ReadableFile->st.file should be NULL");
    }

    self->fd = fopen(CSTRING(self->path), CSTRING(mode));

    if (self->fd == NULL) {
        lk_vm_raise_errno(VM);
    }
}

void lk_file_write_str(lk_file_t *self, lk_str_t *text) {
    if (self->fd == NULL) {
        BUG("WritableFile->st.file should NEVER be NULL");
    }
    vec_print_tostream(VEC(text), self->fd);
}

// info
lk_bool_t *lk_file_is_directory(lk_file_t *self) {
    struct stat info;
    return stat(CSTRING(self->path), &info) == 0 && S_ISDIR(info.st_mode) ? TRUE : FALSE;
}

lk_bool_t *lk_file_is_executable(lk_file_t *self) {
    return access(CSTRING(self->path), X_OK) == 0 ? TRUE : FALSE;
}

lk_bool_t *lk_file_is_exists(lk_file_t *self) {
    return access(CSTRING(self->path), F_OK) == 0 ? TRUE : FALSE;
}

lk_str_t *lk_file_read_num(lk_file_t *self, lk_num_t *length) {
    if (self->fd == NULL) {
        BUG("ReadableFile->st.file should NEVER be NULL");

    } else {
        vec_t *c = vec_str_alloc_from_file_with_length(self->fd, CNUMBER(length));
        return c != NULL ? lk_str_new_from_darray(VM, c) : LK_STRING(NIL);
    }
}

lk_str_t *lk_file_read_all(lk_file_t *self) {
    if (self->fd == NULL) {
        BUG("ReadableFile->st.file should NEVER be NULL");

    } else {
        vec_t *c = vec_str_alloc_from_file(self->fd);
        return c != NULL ? lk_str_new_from_darray(VM, c) : LK_STRING(NIL);
    }
}

lk_str_t *lk_file_read_until_char(lk_file_t *self, lk_char_t *until) {
    if (self->fd == NULL) {
        BUG("ReadableFile->st.file should NEVER be NULL");

    } else {
        vec_t *c = vec_str_alloc_from_file_until_char(self->fd, CHAR(until));
        return c != NULL ? lk_str_new_from_darray(VM, c) : LK_STRING(NIL);
    }
}

lk_str_t *lk_file_read_until_charset(lk_file_t *self, lk_charset_t *until) {
    if (self->fd == NULL) {
        BUG("ReadableFile->st.file should NEVER be NULL");

    } else {
        vec_t *c = vec_str_alloc_from_file_until_charset(self->fd, CHARSET(until));
        return c != NULL ? lk_str_new_from_darray(VM, c) : LK_STRING(NIL);
    }
}

lk_bool_t *lk_file_is_readable(lk_file_t *self) {
    return access(CSTRING(self->path), R_OK) == 0 ? TRUE : FALSE;
}

lk_num_t *lk_file_size(lk_file_t *self) {
    struct stat info;

    if (stat(CSTRING(self->path), &info) != 0) {
        lk_vm_raise_errno(VM);
    }

    return lk_num_new(VM, (double)info.st_size);
}

lk_bool_t *lk_file_is_writable(lk_file_t *self) {
    return access(CSTRING(self->path), W_OK) == 0 ? TRUE : FALSE;
}

// bind all c funcs to lk equiv
void lk_file_lib_init(lk_vm_t *vm) {
    lk_obj_t *file = vm->t_file, *str = vm->t_str, *num = vm->t_num, *ch = vm->t_char, *charset = vm->t_charset;
    lk_global_set("File", file);

    // props
    lk_obj_set_cfield(file, "path", str, offsetof(lk_file_t, path));
    lk_obj_set_cfield(file, "name", str, offsetof(lk_file_t, name));

    // new
    lk_obj_set_cfunc_cvoid(file, "init!", lk_file_init_str, str, NULL);

    // update
    lk_obj_set_cfunc_cvoid(file, "close!", lk_file_close, NULL);
    lk_obj_set_cfunc_cvoid(file, "delete!", lk_file_delete, NULL);
    lk_obj_set_cfunc_cvoid(file, "flush!", lk_file_flush, NULL);
    lk_obj_set_cfunc_cvoid(file, "move!", lk_file_move_str, str, NULL);
    lk_obj_set_cfunc_cvoid(file, "open", lk_file_open, str, NULL);
    lk_obj_set_cfunc_cvoid(file, "write", lk_file_write_str, str, NULL);

    // info
    lk_obj_set_cfunc_creturn(file, "directory?", lk_file_is_directory, NULL);
    lk_obj_set_cfunc_creturn(file, "executable?", lk_file_is_executable, NULL);
    lk_obj_set_cfunc_creturn(file, "exists?", lk_file_is_exists, NULL);
    lk_obj_set_cfunc_creturn(file, "read", lk_file_read_num, num, NULL);
    lk_obj_set_cfunc_creturn(file, "readAll", lk_file_read_all, NULL);
    lk_obj_set_cfunc_creturn(file, "readUntil", lk_file_read_until_char, ch, NULL);
    lk_obj_set_cfunc_creturn(file, "readUntil", lk_file_read_until_charset, charset, NULL);
    lk_obj_set_cfunc_creturn(file, "readable?", lk_file_is_readable, NULL);
    lk_obj_set_cfunc_creturn(file, "size", lk_file_size, NULL);
    lk_obj_set_cfunc_creturn(file, "writable?", lk_file_is_writable, NULL);

    // standard pipes
    lk_global_set("STDIN", vm->t_stdin);
    lk_global_set("STDOUT", vm->t_stdout);
    lk_global_set("STDERR", vm->t_stderr);
}
