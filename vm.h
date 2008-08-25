#ifndef LK_VM_H
#define LK_VM_H

/* generic lib for handling common types of data */
#include "types.h"
#include <setjmp.h>

/* actual def - add header to above #include's on lk_vm_t change */
struct lk_vm {
    struct lk_rsrcchain {
        uint8_t              isstring;
        lk_string_t         *rsrc;
        struct lk_rsrcchain *prev;
    } *rsrc;
    struct lk_rescue {
        jmp_buf              buf;
        struct lk_rescue    *prev;
        struct lk_rsrcchain *rsrc;
    } *rescue;
    lk_instr_t *currinstr;
    lk_scope_t *currentScope;
    lk_error_t *lasterror;
    lk_gc_t *gc;
    lk_scope_t *global;

    /* freq used primitive types */
    /* bool     */ lk_object_t *t_nil, *t_bool, *t_true, *t_false;
    /* char     */ lk_object_t *t_char;
    /* charset  */ lk_object_t *t_charset;
    /* error    */ lk_object_t *t_error;
    /* file     */ lk_object_t *t_file, *t_folder, *t_stdin, *t_stdout, *t_stderr;
    /* vector   */ lk_object_t *t_vector;
    /* fixnum   */ lk_object_t *t_number;
    /* scope    */ lk_object_t *t_scope;
    /* func     */ lk_object_t *t_func, *t_sig, *t_kfunc, *t_cfunc, *t_gfunc;
    /* seq      */ lk_object_t *t_seq;
    /* instr    */ lk_object_t *t_instr;
    /* list     */ lk_object_t *t_list;
    /* map      */ lk_object_t *t_map;
    /* obj      */ lk_object_t *t_object;
    /* parser   */ lk_object_t *t_parser, *t_prec;
    /* socket   */ lk_object_t *t_socket;
    /* string   */ lk_object_t *t_string;
    /* vm       */ lk_object_t *t_vm;

    /* freq used strings */
    lk_string_t *str_type;
    lk_string_t *str_forward;
    lk_string_t *str_rescue;
    lk_string_t *str_onassign;
    lk_string_t *str_at;
    lk_string_t *str_filesep;

    /* statistics */
    struct {
        long totalInstructions;
        long totalScopes;
        long recycledScopes;
    } stat;
};

/* ext map */
void lk_vm_typeinit(lk_vm_t *vm);
void lk_vm_libinit(lk_vm_t *vm);

/* new */
lk_vm_t *lk_vm_new(void);
void lk_vm_free(lk_vm_t *self);

/* eval */
lk_scope_t *lk_vm_evalfile(lk_vm_t *self, const char *file, const char *base);
lk_scope_t *lk_vm_evalstring(lk_vm_t *self, const char *code);
void lk_vm_doevalfunc(lk_vm_t *vm);
void lk_vm_raisecstr(lk_vm_t *self, const char *message, ...) __attribute__((noreturn));
void lk_vm_raiseerrno(lk_vm_t *self) __attribute__((noreturn));
void lk_vm_raiseerror(lk_vm_t *self, lk_error_t *error) __attribute__((noreturn));
void lk_vm_exit(lk_vm_t *self) __attribute__((noreturn));
void lk_vm_abort(lk_vm_t *self, lk_error_t *error) __attribute__((noreturn));
#endif
