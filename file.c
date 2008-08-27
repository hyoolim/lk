#include "ext.h"
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#define PATH(self) (LK_FILE(self)->path)
#define FD(self) (LK_FILE(self)->file)

/* type */
static LK_OBJ_DEFMARKFUNC(mark_file) {
    mark(LK_OBJ(PATH(self)));
}
static void free_file(lk_obj_t *self) {
    if(FD(self) != NULL) fclose(FD(self));
}
void lk_file_typeinit(lk_vm_t *vm) {
    vm->t_file = lk_obj_allocWithSize(vm->t_obj, sizeof(lk_file_t));
    lk_obj_setmarkfunc(vm->t_file, mark_file);
    lk_obj_setfreefunc(vm->t_file, free_file);
    vm->t_stdin = lk_obj_alloc(vm->t_file);
    FD(vm->t_stdin) = stdin;
    vm->t_stdout = lk_obj_alloc(vm->t_file);
    FD(vm->t_stdout) = stdout;
    vm->t_stderr = lk_obj_alloc(vm->t_file);
    FD(vm->t_stderr) = stderr;
}

/* update */
void lk_file_close(lk_obj_t *self) {
    FILE *fd = FD(self);
    if(fd != NULL) {
        if(fclose(fd) != 0) lk_vm_raiseerrno(VM);
        FD(self) = NULL;
    }
}
void lk_file_delete(lk_obj_t *self) {
    if(remove(CSTRING(PATH(self))) != 0) lk_vm_raiseerrno(VM);
}
void lk_file_flush(lk_obj_t *self) {
    FILE *fd = FD(self);
    if(fd != NULL && fflush(fd) != 0) {
        lk_vm_raiseerrno(VM);
    }
}
void lk_file_init_str(lk_obj_t *self, lk_str_t *path) {
    PATH(self) = path;
}
void lk_file_move_str(lk_obj_t *self, lk_str_t *dest) {
    if(rename(CSTRING(PATH(self)), CSTRING(dest)) != 0) lk_vm_raiseerrno(VM);
}
void lk_file_openforreading(lk_obj_t *self) {
    const char *path = CSTRING(PATH(self));
    if(FD(self) != NULL) BUG("ReadableFile->st.file should be NULL");
    FD(self) = fopen(path, "rb");
    if(FD(self) == NULL) lk_vm_raiseerrno(VM);
}
void lk_file_openforwriting(lk_obj_t *self) {
    const char *path = CSTRING(PATH(self));
    if(FD(self) != NULL) BUG("WritableFile->st.file should be NULL");
    FD(self) = fopen(path, "wb");
    if(FD(self) == NULL) lk_vm_raiseerrno(VM);
}
void lk_file_write_str(lk_obj_t *self, lk_str_t *text) {
    FILE *fd = FD(self);
    if(fd == NULL) BUG("WritableFile->st.file should NEVER be NULL");
    darray_printToStream(DARRAY(text), fd);
}

/* info */
lk_bool_t *lk_file_isdirectory(lk_obj_t *self) {
    struct stat fileInfo;
    return stat(CSTRING(PATH(self)), &fileInfo) == 0 && S_ISDIR(fileInfo.st_mode) ? TRUE : FALSE;
}
lk_bool_t *lk_file_isexecutable(lk_obj_t *self) {
    return access(CSTRING(PATH(self)), X_OK) == 0 ? TRUE : FALSE;
}
lk_bool_t *lk_file_isexists(lk_obj_t *self) {
    return access(CSTRING(PATH(self)), F_OK) == 0 ? TRUE : FALSE;
}
lk_str_t *lk_file_read_num(lk_obj_t *self, lk_num_t *length) {
    FILE *f = FD(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        darray_t *c = darray_allocfromfile(f, CNUMBER(length));
        return c != NULL ? lk_str_newFromDArray(VM, c) : LK_STRING(NIL);
    }
}
lk_str_t *lk_file_readall(lk_obj_t *self) {
    FILE *f = FD(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        darray_t *c = str_allocfromfile(f);
        return c != NULL ? lk_str_newFromDArray(VM, c) : LK_STRING(NIL);
    }
}
lk_str_t *lk_file_readuntil_char(lk_obj_t *self, lk_char_t *until) {
    FILE *f = FD(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        darray_t *c = darray_allocFromFileUntilChar(f, CHAR(until));
        return c != NULL ? lk_str_newFromDArray(VM, c) : LK_STRING(NIL);
    }
}
lk_str_t *lk_file_readuntil_charset(lk_obj_t *self, lk_charset_t *until) {
    FILE *f = FD(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        darray_t *c = darray_allocFromFileUntilCharSet(f, CHARSET(until));
        return c != NULL ? lk_str_newFromDArray(VM, c) : LK_STRING(NIL);
    }
}
lk_bool_t *lk_file_isreadable(lk_obj_t *self) {
    return access(CSTRING(PATH(self)), R_OK) == 0 ? TRUE : FALSE;
}
lk_num_t *lk_file_size(lk_obj_t *self) {
    struct stat fileInfo;
    if(stat(CSTRING(PATH(self)), &fileInfo) != 0) {
        lk_vm_raiseerrno(VM);
    }
    return lk_num_new(VM, fileInfo.st_size);
}
lk_bool_t *lk_file_iswritable(lk_obj_t *self) {
    return access(CSTRING(PATH(self)), W_OK) == 0 ? TRUE : FALSE;
}

/* bind all c funcs to lk equiv */
void lk_file_libinit(lk_vm_t *vm) {
    lk_obj_t *file = vm->t_file, *str = vm->t_str, *num = vm->t_num, *ch = vm->t_char, *charset = vm->t_charset;
    lk_lib_setGlobal("File", file);

    /* props */
    lk_lib_setCField(file, "path", str, offsetof(lk_file_t, path));

    /* update */
    lk_obj_set_cfunc_cvoid(file, "close!", lk_file_close, NULL);
    lk_obj_set_cfunc_cvoid(file, "delete!", lk_file_delete, NULL);
    lk_obj_set_cfunc_cvoid(file, "flush!", lk_file_flush, NULL);
    lk_obj_set_cfunc_cvoid(file, "init!", lk_file_init_str, str, NULL);
    lk_obj_set_cfunc_cvoid(file, "move!", lk_file_move_str, str, NULL);
    lk_obj_set_cfunc_cvoid(file, "openForReading", lk_file_openforreading, NULL);
    lk_obj_set_cfunc_cvoid(file, "openForWriting", lk_file_openforwriting, NULL);
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
    lk_lib_setGlobal("STDIN", vm->t_stdin);
    lk_lib_setGlobal("STDOUT", vm->t_stdout);
    lk_lib_setGlobal("STDERR", vm->t_stderr);
}
