#ifndef LK_VM_H
#define LK_VM_H

// generic lib for handling common types of data
#include "types.h"
#include <setjmp.h>

// actual def - add header to above #include's on lk_vm_t change
struct lk_vm {
    struct lk_rsrcchain {
        bool isstr;
        lk_str_t *rsrc;
        struct lk_rsrcchain *prev;
    } *rsrc;
    struct lk_rescue {
        jmp_buf buf;
        struct lk_rescue *prev;
        struct lk_rsrcchain *rsrc;
    } *rescue;
    lk_instr_t *currinstr;
    lk_scope_t *currscope;
    lk_err_t *lasterr;
    lk_gc_t *gc;
    lk_scope_t *global;

    // freq used primitive types
    lk_obj_t *t_nil, *t_bool, *t_true, *t_false; // bool
    lk_obj_t *t_char;                              // char
    lk_obj_t *t_charset;                           // charset
    lk_obj_t *t_dl;                                // dl
    lk_obj_t *t_err;                               // err
    lk_obj_t *t_file, *t_dir, *t_stdin, *t_stdout, *t_stderr; // file
    lk_obj_t *t_vec;                               // vec
    lk_obj_t *t_num;                               // fixnum
    lk_obj_t *t_scope;                             // scope
    lk_obj_t *t_func, *t_sig, *t_lfunc, *t_cfunc, *t_gfunc;   // func
    lk_obj_t *t_seq;                               // seq
    lk_obj_t *t_instr;                             // instr
    lk_obj_t *t_list;                              // list
    lk_obj_t *t_map;                               // map
    lk_obj_t *t_obj;                               // obj
    lk_obj_t *t_parser, *t_prec;                   // parser
    lk_obj_t *t_pipe;                              // pipe
    lk_obj_t *t_socket;                            // socket
    lk_obj_t *t_str;                               // str
    lk_obj_t *t_vm;                                // vm

    // freq used strs
    lk_str_t *str_type;
    lk_str_t *str_forward;
    lk_str_t *str_rescue;
    lk_str_t *str_onassign;
    lk_str_t *str_at;
    lk_str_t *str_filesep;

    // statistics
    struct {
        long totalinstrs;
        long totalscopes;
        long recycledscopes;
    } stat;
};

// ext map
void lk_vm_typeinit(lk_vm_t *vm);
void lk_vm_libinit(lk_vm_t *vm);

// new
lk_vm_t *lk_vm_new(void);
void lk_vm_free(lk_vm_t *self);

// eval
lk_scope_t *lk_vm_evalfile(lk_vm_t *self, const char *file, const char *base);
lk_scope_t *lk_vm_evalstr(lk_vm_t *self, const char *code);
void lk_vm_doevalfunc(lk_vm_t *vm);
void lk_vm_raisecstr(lk_vm_t *self, const char *message, ...) __attribute__((noreturn));
void lk_vm_raiseerrno(lk_vm_t *self) __attribute__((noreturn));
void lk_vm_raiseerr(lk_vm_t *self, lk_err_t *err) __attribute__((noreturn));
void lk_vm_exit(lk_vm_t *self) __attribute__((noreturn));
void lk_vm_abort(lk_vm_t *self, lk_err_t *err) __attribute__((noreturn));
#endif
