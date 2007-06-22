#include "file.h"
#include "buffer.h"
#include "char.h"
#include "cset.h"
#include "ext.h"
#include "fixnum.h"
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#define PATH(self) (LK_FILE(self)->path)
#define FILEF(self) (LK_FILE(self)->st.file)
#define DIRF(self) (LK_FILE(self)->st.dir)

/* ext map - types */
static LK_OBJECT_DEFMARKFUNC(mark__file) {
    mark(LK_O(PATH(self)));
}
static LK_OBJECT_DEFFREEFUNC(free__file) {
    /* TODO - account for cases when fclose fails */
    if(FILEF(self) != NULL) fclose(FILEF(self));
}
LK_EXT_DEFINIT(lk_file_extinittypes) {
    vm->t_file = lk_object_allocwithsize(vm->t_object, sizeof(lk_file_t));
    lk_object_setmarkfunc(vm->t_file, mark__file);
    lk_object_setfreefunc(vm->t_file, free__file);
    vm->t_dir = lk_object_alloc(vm->t_file);
    vm->t_rf = lk_object_alloc(vm->t_file);
    vm->t_wf = lk_object_alloc(vm->t_file);
    vm->t_stdin = lk_object_alloc(vm->t_rf);
    vm->t_stdout = lk_object_alloc(vm->t_wf);
    vm->t_stderr = lk_object_alloc(vm->t_wf);
}

/* ext map - funcs */
/* File */
static LK_EXT_DEFCFUNC(directoryQ__file) {
    struct stat s;
    RETURN(stat(CSTR(PATH(self)), &s) == 0 && S_ISDIR(s.st_mode) ? T : F);
}
static LK_EXT_DEFCFUNC(delete__file) {
    if(remove(CSTR(PATH(self))) == 0) RETURN(self);
    else lk_vm_raiseerrno(VM);
}
static LK_EXT_DEFCFUNC(executableQ__file) {
    RETURN(access(CSTR(PATH(self)), X_OK) == 0 ? T : F);
}
static LK_EXT_DEFCFUNC(existsQ__file) {
    RETURN(access(CSTR(PATH(self)), F_OK) == 0 ? T : F);
}
static LK_EXT_DEFCFUNC(init__file_str) {
    PATH(self) = LK_STRING(ARG(0));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(move__file_str) {
    if(rename(CSTR(PATH(self)), CSTR(ARG(0))) == 0) RETURN(self);
    else lk_vm_raiseerrno(VM);
}
static LK_EXT_DEFCFUNC(open__dir);
static LK_EXT_DEFCFUNC(open_d__file) {
    self->co.proto = VM->t_dir;
    GOTO(open__dir);
}
static LK_EXT_DEFCFUNC(open__rf);
static LK_EXT_DEFCFUNC(open_r__file) {
    self->co.proto = VM->t_rf;
    GOTO(open__rf);
}
static LK_EXT_DEFCFUNC(open__wf);
static LK_EXT_DEFCFUNC(open_w__file) {
    self->co.proto = VM->t_wf;
    GOTO(open__wf);
}
static LK_EXT_DEFCFUNC(readableQ__file) {
    RETURN(access(CSTR(PATH(self)), R_OK) == 0 ? T : F);
}
static LK_EXT_DEFCFUNC(size__file) {
    struct stat s;
    if(stat(CSTR(PATH(self)), &s) != 0) lk_vm_raiseerrno(VM);
    RETURN(lk_fi_new(VM, s.st_size));
}
static LK_EXT_DEFCFUNC(writableQ__file) {
    RETURN(access(CSTR(PATH(self)), W_OK) == 0 ? T : F);
}
/* Directory */
static LK_EXT_DEFCFUNC(close__dir) {
    DIR *d = DIRF(self);
    if(d == NULL) BUG("Directory->st.dir should NEVER be NULL");
    else {
        if(closedir(DIRF(self)) == 0) DIRF(self) = NULL;
        else lk_vm_raiseerrno(VM);
    }
    self->co.proto = VM->t_file;
    RETURN(self);
}
static LK_EXT_DEFCFUNC(init__dir_str) {
    PATH(self) = LK_STRING(ARG(0));
    GOTO(open__dir);
}
static LK_EXT_DEFCFUNC(open__dir) {
    const char *path = CSTR(PATH(self));
    if(DIRF(self) != NULL) BUG("Directory->st.dir should be NULL");
    DIRF(self) = opendir(path);
    if(DIRF(self) == NULL) lk_vm_raiseerrno(VM);
    else RETURN(self);
}
static LK_EXT_DEFCFUNC(read__dir) {
    DIR *d = DIRF(self);
    if(d == NULL) BUG("Directory->st.dir should NEVER be NULL");
    else {
        struct dirent *e = readdir(d);
        if(e == NULL) {
            if(errno != 0) lk_vm_raiseerrno(VM);
            RETURN(N);
        } else {
            lk_object_t *f = lk_object_alloc(VM->t_file);
            string_t *p1, *p2, *fs = LIST(VM->str_filesep);
            PATH(f) = lk_string_newfromlist(VM, LIST(PATH(self)));
            p1 = LIST(PATH(f));
            p2 = string_allocfromcstr(e->d_name);
            if(list_findlist(p1, fs, p1->count - fs->count) < 0) {
                list_concat(p1, fs);
            }
            list_concat(p1, p2);
            list_free(p2);
            RETURN(f);
        }
    }
}
/* ReadableFile + WritableFile */
static LK_EXT_DEFCFUNC(close__rwf) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("Readable/WritableFile->st.file should NEVER be NULL");
    else {
        if(fclose(FILEF(self)) == 0) FILEF(self) = NULL;
        else lk_vm_raiseerrno(VM);
    }
    self->co.proto = VM->t_file;
    RETURN(self);
}
/* ReadableFile */
static LK_EXT_DEFCFUNC(init__rf_str) {
    PATH(self) = LK_STRING(ARG(0));
    GOTO(open__rf);
}
static LK_EXT_DEFCFUNC(open__rf) {
    const char *path = CSTR(PATH(self));
    if(FILEF(self) != NULL) BUG("ReadableFile->st.file should be NULL");
    FILEF(self) = fopen(path, "rb");
    if(FILEF(self) == NULL) lk_vm_raiseerrno(VM);
    else RETURN(self);
}
static LK_EXT_DEFCFUNC(read__rf) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        string_t *c = string_allocfromfile(f);
        RETURN(c != NULL ? LK_O(lk_string_newfromlist(VM, c)) : N);
    }
}
static LK_EXT_DEFCFUNC(read__rf_ch) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        string_t *c = string_allocfromfileuntilchar(f, CHAR(ARG(0)));
        RETURN(c != NULL ? LK_O(lk_string_newfromlist(VM, c)) : N);
    }
}
static LK_EXT_DEFCFUNC(read__rf_cset) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        string_t *c = string_allocfromfileuntilcset(f, CSET(ARG(0)));
        RETURN(c != NULL ? LK_O(lk_string_newfromlist(VM, c)) : N);
    }
}
static LK_EXT_DEFCFUNC(read__rf_fi) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("ReadableFile->st.file should NEVER be NULL");
    else {
        list_t *c = list_allocfromfile(f, INT(ARG(0)));
        /* RETURN(c != NULL ? LK_O(lk_buffer_newfromlist(VM, c)) : N); */
        RETURN(c != NULL ? LK_O(lk_string_newfromlist(VM, c)) : N);
    }
}
/* WritableFile */
static LK_EXT_DEFCFUNC(init__wf_str) {
    PATH(self) = LK_STRING(ARG(0));
    GOTO(open__wf);
}
static LK_EXT_DEFCFUNC(flush__wf) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("WritableFile->st.file should NEVER be NULL");
    if(fflush(f) == 0) RETURN(self);
    else lk_vm_raiseerrno(VM);
}
static LK_EXT_DEFCFUNC(open__wf) {
    const char *path = CSTR(PATH(self));
    if(FILEF(self) != NULL) BUG("WritableFile->st.file should be NULL");
    FILEF(self) = fopen(path, "wb");
    if(FILEF(self) == NULL) lk_vm_raiseerrno(VM);
    else RETURN(self);
}
static LK_EXT_DEFCFUNC(write__wf_str) {
    FILE *f = FILEF(self);
    if(f == NULL) BUG("WritableFile->st.file should NEVER be NULL");
    else {
        string_print(LIST(ARG(0)), f);
        RETURN(self);
    }
}
LK_EXT_DEFINIT(lk_file_extinitfuncs) {
    lk_object_t *file = vm->t_file, *dir = vm->t_dir,
                *rf = vm->t_rf, *wf = vm->t_wf,
                *str = vm->t_string, *fi = vm->t_fi,
                *ch = vm->t_char, *cset = vm->t_cset;
    /* File */
    lk_ext_global("File", file);
    lk_ext_cfunc(file, "directory?", directoryQ__file, NULL);
    lk_ext_cfunc(file, "delete", delete__file, NULL);
    lk_ext_cfunc(file, "executable?", executableQ__file, NULL);
    lk_ext_cfunc(file, "exists?", existsQ__file, NULL);
    lk_ext_cfunc(file, "init", init__file_str, str, NULL);
    lk_ext_cfunc(file, "move", move__file_str, str, NULL);
    lk_ext_cfunc(file, "open", open_r__file, NULL);
    lk_ext_cfunc(file, "open_d", open_d__file, NULL);
    lk_ext_cfunc(file, "open_r", open_r__file, NULL);
    lk_ext_cfunc(file, "open_w", open_w__file, NULL);
    lk_ext_cfield(file, "path", str, offsetof(lk_file_t, path));
    lk_ext_cfunc(file, "readable?", readableQ__file, NULL);
    lk_ext_cfunc(file, "size", size__file, NULL);
    lk_ext_cfunc(file, "writable?", writableQ__file, NULL);
    /* Directory */
    lk_ext_global("Directory", dir);
    lk_ext_cfunc(dir, "close", close__dir, NULL);
    lk_ext_cfunc(dir, "init", init__dir_str, str, NULL);
    lk_ext_cfunc(dir, "open", open__dir, NULL);
    lk_ext_cfunc(dir, "read", read__dir, NULL);
    /* ReadableFile */
    lk_ext_global("ReadableFile", rf);
    lk_ext_cfunc(rf, "close", close__rwf, NULL);
    lk_ext_cfunc(rf, "init", init__rf_str, str, NULL);
    lk_ext_cfunc(rf, "open", open__rf, NULL);
    lk_ext_cfunc(rf, "read", read__rf, NULL);
    lk_ext_cfunc(rf, "read", read__rf_ch, ch, NULL);
    lk_ext_cfunc(rf, "read", read__rf_cset, cset, NULL);
    lk_ext_cfunc(rf, "read", read__rf_fi, fi, NULL);
    /* WritableFile */
    lk_ext_global("WritableFile", wf);
    lk_ext_cfunc(wf, "close", close__rwf, NULL);
    lk_ext_cfunc(wf, "init", init__wf_str, str, NULL);
    lk_ext_cfunc(wf, "flush", flush__wf, NULL);
    lk_ext_cfunc(wf, "open", open__wf, NULL);
    lk_ext_cfunc(wf, "write", write__wf_str, str, NULL);
    /* StandardInput */
    lk_ext_global("STANDARD INPUT", vm->t_stdin);
    lk_ext_global("STDIN", vm->t_stdin);
    FILEF(vm->t_stdin) = stdin;
    /* StandardOutput */
    lk_ext_global("STANDARD OUTPUT", vm->t_stdout);
    lk_ext_global("STDOUT", vm->t_stdout);
    FILEF(vm->t_stdout) = stdout;
    /* StandardError */
    lk_ext_global("STANDARD ERROR", vm->t_stderr);
    lk_ext_global("STDERR", vm->t_stderr);
    FILEF(vm->t_stderr) = stderr;
}
