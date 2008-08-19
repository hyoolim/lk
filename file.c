#include "file.h"
#include "char.h"
#include "charset.h"
#include "ext.h"
#include "fixnum.h"
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#define PATH(self) (LK_FILE(self)->path)
#define FILEF(self) (LK_FILE(self)->file)

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark__file) {
    mark(LK_OBJ(PATH(self)));
}
static LK_OBJ_DEFFREEFUNC(free__file) {
    if(FILEF(self) != NULL) fclose(FILEF(self));
}
LK_EXT_DEFINIT(lk_file_extinittypes) {
    vm->t_file = lk_object_allocwithsize(vm->t_obj, sizeof(lk_file_t));
    lk_object_setmarkfunc(vm->t_file, mark__file);
    lk_object_setfreefunc(vm->t_file, free__file);
    vm->t_rf = lk_object_alloc(vm->t_file);
    vm->t_wf = lk_object_alloc(vm->t_file);
    vm->t_stdin = lk_object_alloc(vm->t_rf);
    vm->t_stdout = lk_object_alloc(vm->t_wf);
    vm->t_stderr = lk_object_alloc(vm->t_wf);
}

/* ext map - funcs */
LK_LIB_DEFINECFUNC(close__file) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("Readable/WritableFile->st.file should NEVER be NULL");
    else {
        if(fclose(FILEF(self)) == 0) FILEF(self) = NULL;
        else lk_vm_raiseerrno(VM);
    }
    self->o.parent = VM->t_file;
    RETURN(self);
}
LK_LIB_DEFINECFUNC(directoryQ__file) {
    struct stat s;
    RETURN(stat(CSTRING(PATH(self)), &s) == 0 && S_ISDIR(s.st_mode) ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(delete__file) {
    if(remove(CSTRING(PATH(self))) == 0) RETURN(self);
    else lk_vm_raiseerrno(VM);
}
LK_LIB_DEFINECFUNC(executableQ__file) {
    RETURN(access(CSTRING(PATH(self)), X_OK) == 0 ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(existsQ__file) {
    RETURN(access(CSTRING(PATH(self)), F_OK) == 0 ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(flush__file) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("WritableFile->st.file should NEVER be NULL");
    if(fflush(f) == 0) RETURN(self);
    else lk_vm_raiseerrno(VM);
}
LK_LIB_DEFINECFUNC(init__file_str) {
    PATH(self) = LK_STRING(ARG(0));
    RETURN(self);
}
LK_LIB_DEFINECFUNC(move__file_str) {
    if(rename(CSTRING(PATH(self)), CSTRING(ARG(0))) == 0) RETURN(self);
    else lk_vm_raiseerrno(VM);
}
LK_LIB_DEFINECFUNC(open_r__file) {
    const char *path = CSTRING(PATH(self));
    if(FILEF(self) != NULL) BUG("ReadableFile->st.file should be NULL");
    FILEF(self) = fopen(path, "rb");
    if(FILEF(self) == NULL) lk_vm_raiseerrno(VM);
    else RETURN(self);
}
LK_LIB_DEFINECFUNC(open_w__file) {
    const char *path = CSTRING(PATH(self));
    if(FILEF(self) != NULL) BUG("WritableFile->st.file should be NULL");
    FILEF(self) = fopen(path, "wb");
    if(FILEF(self) == NULL) lk_vm_raiseerrno(VM);
    else RETURN(self);
}
LK_LIB_DEFINECFUNC(read__file) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        darray_t *c = string_allocfromfile(f);
        RETURN(c != NULL ? LK_OBJ(lk_string_newFromDArray(VM, c)) : NIL);
    }
}
LK_LIB_DEFINECFUNC(read__file_ch) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        darray_t *c = darray_allocFromFileUntilChar(f, CHAR(ARG(0)));
        RETURN(c != NULL ? LK_OBJ(lk_string_newFromDArray(VM, c)) : NIL);
    }
}
LK_LIB_DEFINECFUNC(read__file_charset) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        darray_t *c = darray_allocFromFileUntilCharSet(f, CHARSET(ARG(0)));
        RETURN(c != NULL ? LK_OBJ(lk_string_newFromDArray(VM, c)) : NIL);
    }
}
LK_LIB_DEFINECFUNC(read__file_fi) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        darray_t *c = darray_allocfromfile(f, INT(ARG(0)));
        RETURN(c != NULL ? LK_OBJ(lk_string_newFromDArray(VM, c)) : NIL);
    }
}
LK_LIB_DEFINECFUNC(readableQ__file) {
    RETURN(access(CSTRING(PATH(self)), R_OK) == 0 ? TRUE : FALSE);
}
LK_LIB_DEFINECFUNC(size__file) {
    struct stat s;
    if(stat(CSTRING(PATH(self)), &s) != 0) lk_vm_raiseerrno(VM);
    RETURN(lk_fi_new(VM, s.st_size));
}
LK_LIB_DEFINECFUNC(write__file_str) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("WritableFile->st.file should NEVER be NULL");
    else {
        darray_printToStream(DARRAY(ARG(0)), f);
        RETURN(self);
    }
}
LK_LIB_DEFINECFUNC(writableQ__file) {
    RETURN(access(CSTRING(PATH(self)), W_OK) == 0 ? TRUE : FALSE);
}
LK_EXT_DEFINIT(lk_file_extinitfuncs) {
    lk_object_t *file = vm->t_file,
                *rf = vm->t_rf, *wf = vm->t_wf,
                *str = vm->t_string, *fi = vm->t_fi,
                *ch = vm->t_char, *charset = vm->t_charset;
    /* File */
    lk_lib_setGlobal("File", file);
    lk_lib_setCFunc(file, "close", close__file, NULL);
    lk_lib_setCFunc(file, "directory?", directoryQ__file, NULL);
    lk_lib_setCFunc(file, "delete", delete__file, NULL);
    lk_lib_setCFunc(file, "executable?", executableQ__file, NULL);
    lk_lib_setCFunc(file, "exists?", existsQ__file, NULL);
    lk_lib_setCFunc(file, "flush", flush__file, NULL);
    lk_lib_setCFunc(file, "init", init__file_str, str, NULL);
    lk_lib_setCFunc(file, "move", move__file_str, str, NULL);
    lk_lib_setCFunc(file, "open", open_r__file, NULL);
    lk_lib_setCFunc(file, "open_r", open_r__file, NULL);
    lk_lib_setCFunc(file, "open_w", open_w__file, NULL);
    lk_lib_setCField(file, "path", str, offsetof(lk_file_t, path));
    lk_lib_setCFunc(file, "read", read__file, NULL);
    lk_lib_setCFunc(file, "read", read__file_ch, ch, NULL);
    lk_lib_setCFunc(file, "read", read__file_charset, charset, NULL);
    lk_lib_setCFunc(file, "read", read__file_fi, fi, NULL);
    lk_lib_setCFunc(file, "readable?", readableQ__file, NULL);
    lk_lib_setCFunc(file, "size", size__file, NULL);
    lk_lib_setCFunc(file, "write", write__file_str, str, NULL);
    lk_lib_setCFunc(file, "writable?", writableQ__file, NULL);
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
