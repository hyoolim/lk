#include "ext.h"
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#define PATH(self) (LK_FILE(self)->path)
#define FILEF(self) (LK_FILE(self)->file)

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark_file) {
    mark(LK_OBJ(PATH(self)));
}
static void free_file(lk_object_t *self) {
    if(FILEF(self) != NULL) fclose(FILEF(self));
}
void lk_file_typeinit(lk_vm_t *vm) {
    vm->t_file = lk_object_allocWithSize(vm->t_object, sizeof(lk_file_t));
    lk_object_setmarkfunc(vm->t_file, mark_file);
    lk_object_setfreefunc(vm->t_file, free_file);
    vm->t_stdin = lk_object_alloc(vm->t_file);
    vm->t_stdout = lk_object_alloc(vm->t_file);
    vm->t_stderr = lk_object_alloc(vm->t_file);
}

/* ext map - funcs */
static void close_file(lk_object_t *self, lk_scope_t *local) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("Readable/WritableFile->st.file should NEVER be NULL");
    else {
        if(fclose(FILEF(self)) == 0) FILEF(self) = NULL;
        else lk_vm_raiseerrno(VM);
    }
    self->o.parent = VM->t_file;
    RETURN(self);
}
static void directoryQ_file(lk_object_t *self, lk_scope_t *local) {
    struct stat s;
    RETURN(stat(CSTRING(PATH(self)), &s) == 0 && S_ISDIR(s.st_mode) ? TRUE : FALSE);
}
static void delete_file(lk_object_t *self, lk_scope_t *local) {
    if(remove(CSTRING(PATH(self))) == 0) RETURN(self);
    else lk_vm_raiseerrno(VM);
}
static void executableQ_file(lk_object_t *self, lk_scope_t *local) {
    RETURN(access(CSTRING(PATH(self)), X_OK) == 0 ? TRUE : FALSE);
}
static void existsQ_file(lk_object_t *self, lk_scope_t *local) {
    RETURN(access(CSTRING(PATH(self)), F_OK) == 0 ? TRUE : FALSE);
}
static void flush_file(lk_object_t *self, lk_scope_t *local) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("WritableFile->st.file should NEVER be NULL");
    if(fflush(f) == 0) RETURN(self);
    else lk_vm_raiseerrno(VM);
}
static void init_file_str(lk_object_t *self, lk_scope_t *local) {
    PATH(self) = LK_STRING(ARG(0));
    RETURN(self);
}
static void move_file_str(lk_object_t *self, lk_scope_t *local) {
    if(rename(CSTRING(PATH(self)), CSTRING(ARG(0))) == 0) RETURN(self);
    else lk_vm_raiseerrno(VM);
}
static void openForReading_file(lk_object_t *self, lk_scope_t *local) {
    const char *path = CSTRING(PATH(self));
    if(FILEF(self) != NULL) BUG("ReadableFile->st.file should be NULL");
    FILEF(self) = fopen(path, "rb");
    if(FILEF(self) == NULL) lk_vm_raiseerrno(VM);
    else RETURN(self);
}
static void openForWriting_file(lk_object_t *self, lk_scope_t *local) {
    const char *path = CSTRING(PATH(self));
    if(FILEF(self) != NULL) BUG("WritableFile->st.file should be NULL");
    FILEF(self) = fopen(path, "wb");
    if(FILEF(self) == NULL) lk_vm_raiseerrno(VM);
    else RETURN(self);
}
static void read_file(lk_object_t *self, lk_scope_t *local) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        darray_t *c = string_allocfromfile(f);
        RETURN(c != NULL ? LK_OBJ(lk_string_newFromDArray(VM, c)) : NIL);
    }
}
static void read_file_ch(lk_object_t *self, lk_scope_t *local) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        darray_t *c = darray_allocFromFileUntilChar(f, CHAR(ARG(0)));
        RETURN(c != NULL ? LK_OBJ(lk_string_newFromDArray(VM, c)) : NIL);
    }
}
static void read_file_charset(lk_object_t *self, lk_scope_t *local) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        darray_t *c = darray_allocFromFileUntilCharSet(f, CHARSET(ARG(0)));
        RETURN(c != NULL ? LK_OBJ(lk_string_newFromDArray(VM, c)) : NIL);
    }
}
static void read_file_number(lk_object_t *self, lk_scope_t *local) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        darray_t *c = darray_allocfromfile(f, CNUMBER(ARG(0)));
        RETURN(c != NULL ? LK_OBJ(lk_string_newFromDArray(VM, c)) : NIL);
    }
}
static void readableQ_file(lk_object_t *self, lk_scope_t *local) {
    RETURN(access(CSTRING(PATH(self)), R_OK) == 0 ? TRUE : FALSE);
}
static void size_file(lk_object_t *self, lk_scope_t *local) {
    struct stat s;
    if(stat(CSTRING(PATH(self)), &s) != 0) lk_vm_raiseerrno(VM);
    RETURN(lk_number_new(VM, s.st_size));
}
static void write_file_str(lk_object_t *self, lk_scope_t *local) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("WritableFile->st.file should NEVER be NULL");
    else {
        darray_printToStream(DARRAY(ARG(0)), f);
        RETURN(self);
    }
}
static void writableQ_file(lk_object_t *self, lk_scope_t *local) {
    RETURN(access(CSTRING(PATH(self)), W_OK) == 0 ? TRUE : FALSE);
}
void lk_file_libinit(lk_vm_t *vm) {
    lk_object_t *file = vm->t_file,
                *str = vm->t_string, *number = vm->t_number,
                *ch = vm->t_char, *charset = vm->t_charset;
    /* File */
    lk_lib_setGlobal("File", file);
    lk_object_set_cfunc_lk(file, "close", close_file, NULL);
    lk_object_set_cfunc_lk(file, "directory?", directoryQ_file, NULL);
    lk_object_set_cfunc_lk(file, "delete", delete_file, NULL);
    lk_object_set_cfunc_lk(file, "executable?", executableQ_file, NULL);
    lk_object_set_cfunc_lk(file, "exists?", existsQ_file, NULL);
    lk_object_set_cfunc_lk(file, "flush", flush_file, NULL);
    lk_object_set_cfunc_lk(file, "init!", init_file_str, str, NULL);
    lk_object_set_cfunc_lk(file, "move", move_file_str, str, NULL);
    lk_object_set_cfunc_lk(file, "openForReading", openForReading_file, NULL);
    lk_object_set_cfunc_lk(file, "openForWriting", openForWriting_file, NULL);
    lk_lib_setCField(file, "path", str, offsetof(lk_file_t, path));
    lk_object_set_cfunc_lk(file, "read", read_file, NULL);
    lk_object_set_cfunc_lk(file, "read", read_file_ch, ch, NULL);
    lk_object_set_cfunc_lk(file, "read", read_file_charset, charset, NULL);
    lk_object_set_cfunc_lk(file, "read", read_file_number, number, NULL);
    lk_object_set_cfunc_lk(file, "readable?", readableQ_file, NULL);
    lk_object_set_cfunc_lk(file, "size", size_file, NULL);
    lk_object_set_cfunc_lk(file, "write", write_file_str, str, NULL);
    lk_object_set_cfunc_lk(file, "writable?", writableQ_file, NULL);
    /* StandardInput */
    lk_lib_setGlobal("STANDARD INPUT", vm->t_stdin);
    lk_lib_setGlobal("STDIN", vm->t_stdin);
    FILEF(vm->t_stdin) = stdin;
    /* StandardOutput */
    lk_lib_setGlobal("STANDARD OUTPUT", vm->t_stdout);
    lk_lib_setGlobal("STDOUT", vm->t_stdout);
    FILEF(vm->t_stdout) = stdout;
    /* StandardError */
    lk_lib_setGlobal("STANDARD ERROR", vm->t_stderr);
    lk_lib_setGlobal("STDERR", vm->t_stderr);
    FILEF(vm->t_stderr) = stderr;
}
