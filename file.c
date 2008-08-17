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
    /* TODO - account for cases when fclose fails */
    /*
    if(FILEF(self) != NULL) fclose(FILEF(self));
    */
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
/* File */
LK_LIBRARY_DEFINECFUNCTION(directoryQ__file) {
    struct stat s;
    RETURN(stat(CSTR(PATH(self)), &s) == 0 && S_ISDIR(s.st_mode) ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(delete__file) {
    if(remove(CSTR(PATH(self))) == 0) RETURN(self);
    else lk_vm_raiseerrno(VM);
}
LK_LIBRARY_DEFINECFUNCTION(executableQ__file) {
    RETURN(access(CSTR(PATH(self)), X_OK) == 0 ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(existsQ__file) {
    RETURN(access(CSTR(PATH(self)), F_OK) == 0 ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(init__file_str) {
    PATH(self) = LK_STRING(ARG(0));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(move__file_str) {
    if(rename(CSTR(PATH(self)), CSTR(ARG(0))) == 0) RETURN(self);
    else lk_vm_raiseerrno(VM);
}
LK_LIBRARY_DEFINECFUNCTION(open__dir);
LK_LIBRARY_DEFINECFUNCTION(open__rf);
LK_LIBRARY_DEFINECFUNCTION(open_r__file) {
    self->obj.parent = VM->t_rf;
    GOTO(open__rf);
}
LK_LIBRARY_DEFINECFUNCTION(open__wf);
LK_LIBRARY_DEFINECFUNCTION(open_w__file) {
    self->obj.parent = VM->t_wf;
    GOTO(open__wf);
}
LK_LIBRARY_DEFINECFUNCTION(readableQ__file) {
    RETURN(access(CSTR(PATH(self)), R_OK) == 0 ? T : F);
}
LK_LIBRARY_DEFINECFUNCTION(size__file) {
    struct stat s;
    if(stat(CSTR(PATH(self)), &s) != 0) lk_vm_raiseerrno(VM);
    RETURN(lk_fi_new(VM, s.st_size));
}
LK_LIBRARY_DEFINECFUNCTION(writableQ__file) {
    RETURN(access(CSTR(PATH(self)), W_OK) == 0 ? T : F);
}
/* ReadableFile + WritableFile */
LK_LIBRARY_DEFINECFUNCTION(close__rwf) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("Readable/WritableFile->st.file should NEVER be NULL");
    else {
        if(fclose(FILEF(self)) == 0) FILEF(self) = NULL;
        else lk_vm_raiseerrno(VM);
    }
    self->obj.parent = VM->t_file;
    RETURN(self);
}
/* ReadableFile */
LK_LIBRARY_DEFINECFUNCTION(init__rf_str) {
    PATH(self) = LK_STRING(ARG(0));
    GOTO(open__rf);
}
LK_LIBRARY_DEFINECFUNCTION(open__rf) {
    const char *path = CSTR(PATH(self));
    if(FILEF(self) != NULL) BUG("ReadableFile->st.file should be NULL");
    FILEF(self) = fopen(path, "rb");
    if(FILEF(self) == NULL) lk_vm_raiseerrno(VM);
    else RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(read__rf) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        array_t *c = string_allocfromfile(f);
        RETURN(c != NULL ? LK_OBJ(lk_string_newfromlist(VM, c)) : N);
    }
}
LK_LIBRARY_DEFINECFUNCTION(read__rf_ch) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        array_t *c = array_allocFromFileUntilChar(f, CHAR(ARG(0)));
        RETURN(c != NULL ? LK_OBJ(lk_string_newfromlist(VM, c)) : N);
    }
}
LK_LIBRARY_DEFINECFUNCTION(read__rf_charset) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        array_t *c = array_allocFromFileUntilCSet(f, CHARSET(ARG(0)));
        RETURN(c != NULL ? LK_OBJ(lk_string_newfromlist(VM, c)) : N);
    }
}
LK_LIBRARY_DEFINECFUNCTION(read__rf_fi) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        array_t *c = array_allocfromfile(f, INT(ARG(0)));
        RETURN(c != NULL ? LK_OBJ(lk_string_newfromlist(VM, c)) : N);
    }
}
/* WritableFile */
LK_LIBRARY_DEFINECFUNCTION(init__wf_str) {
    PATH(self) = LK_STRING(ARG(0));
    GOTO(open__wf);
}
LK_LIBRARY_DEFINECFUNCTION(flush__wf) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("WritableFile->st.file should NEVER be NULL");
    if(fflush(f) == 0) RETURN(self);
    else lk_vm_raiseerrno(VM);
}
LK_LIBRARY_DEFINECFUNCTION(open__wf) {
    const char *path = CSTR(PATH(self));
    if(FILEF(self) != NULL) BUG("WritableFile->st.file should be NULL");
    FILEF(self) = fopen(path, "wb");
    if(FILEF(self) == NULL) lk_vm_raiseerrno(VM);
    else RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(write__wf_str) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("WritableFile->st.file should NEVER be NULL");
    else {
        string_print(LIST(ARG(0)), f);
        RETURN(self);
    }
}
LK_EXT_DEFINIT(lk_file_extinitfuncs) {
    lk_object_t *file = vm->t_file,
                *rf = vm->t_rf, *wf = vm->t_wf,
                *str = vm->t_string, *fi = vm->t_fi,
                *ch = vm->t_char, *charset = vm->t_charset;
    /* File */
    lk_library_setGlobal("File", file);
    lk_library_setCFunction(file, "directory?", directoryQ__file, NULL);
    lk_library_setCFunction(file, "delete", delete__file, NULL);
    lk_library_setCFunction(file, "executable?", executableQ__file, NULL);
    lk_library_setCFunction(file, "exists?", existsQ__file, NULL);
    lk_library_setCFunction(file, "init", init__file_str, str, NULL);
    lk_library_setCFunction(file, "move", move__file_str, str, NULL);
    lk_library_setCFunction(file, "open", open_r__file, NULL);
    lk_library_setCFunction(file, "open_r", open_r__file, NULL);
    lk_library_setCFunction(file, "open_w", open_w__file, NULL);
    lk_library_cfield(file, "path", str, offsetof(lk_file_t, path));
    lk_library_setCFunction(file, "readable?", readableQ__file, NULL);
    lk_library_setCFunction(file, "size", size__file, NULL);
    lk_library_setCFunction(file, "writable?", writableQ__file, NULL);
    /* ReadableFile */
    lk_library_setGlobal("ReadableFile", rf);
    lk_library_setCFunction(rf, "close", close__rwf, NULL);
    lk_library_setCFunction(rf, "init", init__rf_str, str, NULL);
    lk_library_setCFunction(rf, "open", open__rf, NULL);
    lk_library_setCFunction(rf, "read", read__rf, NULL);
    lk_library_setCFunction(rf, "read", read__rf_ch, ch, NULL);
    lk_library_setCFunction(rf, "read", read__rf_charset, charset, NULL);
    lk_library_setCFunction(rf, "read", read__rf_fi, fi, NULL);
    /* WritableFile */
    lk_library_setGlobal("WritableFile", wf);
    lk_library_setCFunction(wf, "close", close__rwf, NULL);
    lk_library_setCFunction(wf, "init", init__wf_str, str, NULL);
    lk_library_setCFunction(wf, "flush", flush__wf, NULL);
    lk_library_setCFunction(wf, "open", open__wf, NULL);
    lk_library_setCFunction(wf, "write", write__wf_str, str, NULL);
    /* StandardInput */
    lk_library_setGlobal("STANDARD INPUT", vm->t_stdin);
    lk_library_setGlobal("STDIN", vm->t_stdin);
    FILEF(vm->t_stdin) = stdin;
    /* StandardOutput */
    lk_library_setGlobal("STANDARD OUTPUT", vm->t_stdout);
    lk_library_setGlobal("STDOUT", vm->t_stdout);
    FILEF(vm->t_stdout) = stdout;
    /* StandardError */
    lk_library_setGlobal("STANDARD ERROR", vm->t_stderr);
    lk_library_setGlobal("STDERR", vm->t_stderr);
    FILEF(vm->t_stderr) = stderr;
}
