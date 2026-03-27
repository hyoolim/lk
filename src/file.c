#include "lib.h"
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

// type
static void close_fd(void *ptr) {
    fclose((FILE *)ptr);
}

void lk_file_type_init(lk_vm_t *vm) {
    vm->t_file = lk_obj_alloc_type(vm->t_obj, sizeof(lk_obj_t));
    vm->t_stdin = lk_obj_alloc(vm->t_file);
    vm->t_stdout = lk_obj_alloc(vm->t_file);
    vm->t_stderr = lk_obj_alloc(vm->t_file);
    lk_obj_set_slot_by_cstr(vm->t_stdin, "fd", NULL, LK_OBJ(lk_cptr_new(vm, stdin, NULL)));
    lk_obj_set_slot_by_cstr(vm->t_stdout, "fd", NULL, LK_OBJ(lk_cptr_new(vm, stdout, NULL)));
    lk_obj_set_slot_by_cstr(vm->t_stderr, "fd", NULL, LK_OBJ(lk_cptr_new(vm, stderr, NULL)));
}

// new
lk_obj_t *lk_file_new_with_path(lk_vm_t *vm, lk_str_t *path) {
    lk_obj_t *self = lk_obj_alloc(vm->t_file);
    lk_file_init(self, path);
    return self;
}

void lk_file_init(lk_obj_t *self, lk_str_t *path) {
    lk_str_t *filepath = path;
    lk_str_t *name;
    int at = 0, nextat;

    if (vec_str_get(VEC(path), 0) != '/') {
        char buf[1000];

        if (getcwd(buf, 1000) != NULL) {
            lk_str_t *abs = lk_str_new_from_cstr(VM, buf);
            vec_concat(VEC(abs), VEC(VM->str_filesep));
            vec_concat(VEC(abs), VEC(path));
            filepath = abs;
        }
    }

    while ((nextat = vec_str_find(VEC(filepath), '/', at)) > -1) {
        at = nextat + 1;
    }

    name = lk_str_new_from_darray(VM, VEC(filepath));
    vec_offset(VEC(name), at);

    lk_obj_set_slot_by_cstr(self, "path", NULL, LK_OBJ(filepath));
    lk_obj_set_slot_by_cstr(self, "name", NULL, LK_OBJ(name));
}

// update
void lk_file_close(lk_obj_t *self) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr != NULL && cptr->ptr != NULL) {
        if (fclose((FILE *)cptr->ptr) != 0)
            lk_vm_raise_errno(VM);

        cptr->ptr = NULL;
    }
}

void lk_file_delete(lk_obj_t *self) {
    lk_str_t *path = LK_STRING(lk_obj_get_value_by_cstr(self, "path"));

    if (remove(CSTRING(path)) != 0)
        lk_vm_raise_errno(VM);
}

void lk_file_flush(lk_obj_t *self) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr != NULL && cptr->ptr != NULL && fflush((FILE *)cptr->ptr) != 0)
        lk_vm_raise_errno(VM);
}

void lk_file_move_str(lk_obj_t *self, lk_str_t *dest) {
    lk_str_t *path = LK_STRING(lk_obj_get_value_by_cstr(self, "path"));

    if (rename(CSTRING(path), CSTRING(dest)) != 0)
        lk_vm_raise_errno(VM);
}

void lk_file_open(lk_obj_t *self, lk_str_t *mode) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));
    lk_str_t *path = LK_STRING(lk_obj_get_value_by_cstr(self, "path"));
    FILE *fd;

    if (cptr != NULL && cptr->ptr != NULL)
        BUG("ReadableFile->st.file should be NULL");

    fd = fopen(CSTRING(path), CSTRING(mode));

    if (fd == NULL)
        lk_vm_raise_errno(VM);

    lk_obj_set_slot_by_cstr(self, "fd", NULL, LK_OBJ(lk_cptr_new(VM, fd, close_fd)));
}

void lk_file_write_str(lk_obj_t *self, lk_str_t *text) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr == NULL || cptr->ptr == NULL)
        BUG("WritableFile->st.file should NEVER be NULL");

    vec_print_tostream(VEC(text), (FILE *)cptr->ptr);
}

// info
lk_bool_t *lk_file_is_directory(lk_obj_t *self) {
    lk_str_t *path = LK_STRING(lk_obj_get_value_by_cstr(self, "path"));
    struct stat info;

    return stat(CSTRING(path), &info) == 0 && S_ISDIR(info.st_mode) ? TRUE : FALSE;
}

lk_bool_t *lk_file_is_executable(lk_obj_t *self) {
    lk_str_t *path = LK_STRING(lk_obj_get_value_by_cstr(self, "path"));

    return access(CSTRING(path), X_OK) == 0 ? TRUE : FALSE;
}

lk_bool_t *lk_file_is_exists(lk_obj_t *self) {
    lk_str_t *path = LK_STRING(lk_obj_get_value_by_cstr(self, "path"));

    return access(CSTRING(path), F_OK) == 0 ? TRUE : FALSE;
}

lk_str_t *lk_file_read_num(lk_obj_t *self, lk_num_t *length) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr == NULL || cptr->ptr == NULL) {
        BUG("ReadableFile->st.file should NEVER be NULL");

    } else {
        vec_t *c = vec_str_alloc_from_file_with_length((FILE *)cptr->ptr, CNUMBER(length));
        return c != NULL ? lk_str_new_from_darray(VM, c) : LK_STRING(NIL);
    }
}

lk_str_t *lk_file_read_all(lk_obj_t *self) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr == NULL || cptr->ptr == NULL) {
        BUG("ReadableFile->st.file should NEVER be NULL");

    } else {
        vec_t *c = vec_str_alloc_from_file((FILE *)cptr->ptr);
        return c != NULL ? lk_str_new_from_darray(VM, c) : LK_STRING(NIL);
    }
}

lk_str_t *lk_file_read_until_char(lk_obj_t *self, lk_char_t *until) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr == NULL || cptr->ptr == NULL) {
        BUG("ReadableFile->st.file should NEVER be NULL");

    } else {
        vec_t *c = vec_str_alloc_from_file_until_char((FILE *)cptr->ptr, CHAR(until));
        return c != NULL ? lk_str_new_from_darray(VM, c) : LK_STRING(NIL);
    }
}

lk_str_t *lk_file_read_until_charset(lk_obj_t *self, lk_charset_t *until) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr == NULL || cptr->ptr == NULL) {
        BUG("ReadableFile->st.file should NEVER be NULL");

    } else {
        vec_t *c = vec_str_alloc_from_file_until_charset((FILE *)cptr->ptr, CHARSET(until));
        return c != NULL ? lk_str_new_from_darray(VM, c) : LK_STRING(NIL);
    }
}

lk_bool_t *lk_file_is_readable(lk_obj_t *self) {
    lk_str_t *path = LK_STRING(lk_obj_get_value_by_cstr(self, "path"));

    return access(CSTRING(path), R_OK) == 0 ? TRUE : FALSE;
}

lk_num_t *lk_file_size(lk_obj_t *self) {
    lk_str_t *path = LK_STRING(lk_obj_get_value_by_cstr(self, "path"));
    struct stat info;

    if (stat(CSTRING(path), &info) != 0)
        lk_vm_raise_errno(VM);

    return lk_num_new(VM, (double)info.st_size);
}

lk_bool_t *lk_file_is_writable(lk_obj_t *self) {
    lk_str_t *path = LK_STRING(lk_obj_get_value_by_cstr(self, "path"));

    return access(CSTRING(path), W_OK) == 0 ? TRUE : FALSE;
}

// bind all c funcs to lk equiv
void lk_file_lib_init(lk_vm_t *vm) {
    lk_obj_t *file = vm->t_file, *str = vm->t_str, *num = vm->t_num, *ch = vm->t_char, *charset = vm->t_charset;

    lk_global_set("File", file);

    // new
    lk_obj_set_cfunc_cvoid(file, "init!", lk_file_init, str, NULL);

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
