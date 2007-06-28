#ifndef LK_VM_H
#define LK_VM_H

/* generic lib for handling common types of data */
#include "_common.h"
#include "_number.h"
#include "_list.h"
#include "_set.h"
#include "_string.h"
#include <setjmp.h>

/* type - see further down below for actual def */
typedef struct lk_vm lk_vm_t;
typedef struct lk_obj lk_obj_t;
#define LK_OBJ(v) ((lk_obj_t *)(v))

/* common data for all lk objs */
#define LK_OBJ_DEFALLOCFUNC(name) void name(lk_obj_t *self, lk_obj_t *proto)
typedef LK_OBJ_DEFALLOCFUNC(lk_tagallocfunc_t);
#define LK_OBJ_DEFMARKFUNC(name) void name(lk_obj_t *self, void (*mark)(lk_obj_t *self))
typedef LK_OBJ_DEFMARKFUNC(lk_tagmarkfunc_t);
#define LK_OBJ_DEFFREEFUNC(name) void name(lk_obj_t *self)
typedef LK_OBJ_DEFFREEFUNC(lk_tagfreefunc_t);
struct lk_tag {
    int                refc;
    lk_vm_t           *vm;
    size_t             size;
    lk_tagallocfunc_t *allocfunc;
    lk_tagmarkfunc_t  *markfunc;
    lk_tagfreefunc_t  *freefunc;
};
struct lk_objgroup {
    lk_obj_t *first;
    lk_obj_t *last;
};
struct lk_common {
    lk_obj_t            *proto;
    list_t              *ancestors;
    set_t               *slots;
    struct lk_tag          *tag;
    struct lk_mark {
        lk_obj_t        *prev;
        lk_obj_t        *next;
        struct lk_objgroup *objgroup;
        uint8_t             isref;
    }                       mark;
};
#define LK_VM(v) ((v)->co.tag->vm)

/* per-vm globals */
#define LK_VM_DEFGLOBAL_PROTO(name) \
    lk_obj_t *lk_global_ ## name(lk_vm_t *vm)
#define LK_VM_DEFGLOBAL(name) \
    static list_t *g_ ## name; \
    LK_VM_DEFGLOBAL_PROTO(name) { return list_getptr(g_ ## name, vm->id); } \
    LK_VM_DEFGLOBAL_PROTO(name) /* for ; */
#define LK_VM_SETGLOBAL(vm, name, v) do { \
    lk_obj_t *g = (v); \
    g_ ## name = list_allocptr(); \
    list_setptr(g_ ## name, (vm)->id, g); \
    list_pushptr((vm)->retained, g); \
    } while(0)
#define LK_VM_GETGLOBAL(vm, name) \
    (lk_global_ ## name(vm))

/* used by ext - can't be in ext.h due to bootstrapping issues */
#define LK_EXT_DEFINIT(name) void name(lk_vm_t *vm)
typedef LK_EXT_DEFINIT(lk_extinitfunc_t);

/* required primitives */
#include "string.h"
#include "frame.h"
#include "gc.h"
#include "instr.h"
#include "obj.h"
#include "error.h"

/* todo: figure out a way to move this before req primitives */
#define LK_EXT_DEFCFUNC(name) void name(lk_obj_t *self, lk_frame_t *env)
typedef LK_EXT_DEFCFUNC(lk_cfuncfunc_t);

/* actual def - add header to above #include's on lk_vm_t change */
struct lk_obj {
    struct lk_common co;
};
struct lk_vm {
    int id;
    list_t *retained;
    set_t *symbols;
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
    lk_frame_t *currframe;
    lk_error_t *lasterror;
    lk_gc_t *gc;
    lk_frame_t *global;

    /* freq used primitive types */
    /* bool     */ lk_obj_t *t_unknown, *t_bool, *t_true, *t_false,
                               *t_pi, *t_ni;
    /* buffer   */ lk_obj_t *t_buffer;
    /* char     */ lk_obj_t *t_char;
    /* cset     */ lk_obj_t *t_cset;
    /* dict     */ lk_obj_t *t_dict;
    /* error    */ lk_obj_t *t_error;
    /* file     */ lk_obj_t *t_file, *t_dir, *t_rf, *t_wf,
                               *t_stdin, *t_stdout, *t_stderr;
    /* vector   */ lk_obj_t *t_vector;
    /* fixnum   */ lk_obj_t *t_number, *t_int, *t_fi, *t_real, *t_fr;
    /* frame    */ lk_obj_t *t_frame;
    /* func     */ lk_obj_t *t_func, *t_sig, *t_kfunc, *t_cfunc, *t_gfunc;
    /* glist    */ lk_obj_t *t_glist;
    /* gset     */ lk_obj_t *t_gset;
    /* instr    */ lk_obj_t *t_instr;
    /* list     */ lk_obj_t *t_list;
    /* obj      */ lk_obj_t *t_obj;
    /* parser   */ lk_obj_t *t_parser, *t_prec;
    /* string   */ lk_obj_t *t_string;
    /* vm       */ lk_obj_t *t_vm;

    /* freq used strings */
    lk_string_t *str_type;
    lk_string_t *str_forward;
    lk_string_t *str_rescue;
    lk_string_t *str_onassign;
    lk_string_t *str_at;
    lk_string_t *str_filesep;

    /* simple vm stat */
    int cnt_instr;
    int cnt_frame;
    int cnt_recycledframe;
};

/* ext map */
LK_EXT_DEFINIT(lk_vm_extinittypes);
LK_EXT_DEFINIT(lk_vm_extinitfuncs);

/* new */
lk_vm_t *lk_vm_new(void);
lk_vm_t *lk_vm_newwithid(int id);
void lk_vm_free(lk_vm_t *self);

/* eval */
lk_frame_t *lk_vm_evalfile(lk_vm_t *self, const char *file, const char *base);
lk_frame_t *lk_vm_evalstring(lk_vm_t *self, const char *code);
lk_frame_t *lk_vm_prepevalfunc(lk_vm_t *vm);
void lk_vm_doevalfunc(lk_vm_t *vm);
void lk_vm_raisecstr(lk_vm_t *self, const char *message,
                     ...) __attribute__((noreturn));
void lk_vm_raiseerrno(lk_vm_t *self) __attribute__((noreturn));
void lk_vm_raiseerror(lk_vm_t *self,
                      lk_error_t *error) __attribute__((noreturn));
void lk_vm_exit(lk_vm_t *self) __attribute__((noreturn));
void lk_vm_abort(lk_vm_t *self, lk_error_t *error) __attribute__((noreturn));
#endif
