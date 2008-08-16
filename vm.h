#ifndef LK_VM_H
#define LK_VM_H

/* generic lib for handling common types of data */
#include "base/common.h"
#include "base/number.h"
#include "base/list.h"
#include "base/set.h"
#include "base/string.h"
#include <setjmp.h>

/* type - see further down below for actual def */
typedef struct lk_Vm lk_Vm_t;
typedef struct lk_Object lk_Object_t;
#define LK_OBJ(v) ((lk_Object_t *)(v))

/* common data for all lk objs */
#define LK_OBJ_DEFALLOCFUNC(name) void name(lk_Object_t *self, lk_Object_t *parent)
typedef LK_OBJ_DEFALLOCFUNC(lk_Tagallocfunc_t);
#define LK_OBJ_DEFMARKFUNC(name) void name(lk_Object_t *self, void (*mark)(lk_Object_t *self))
typedef LK_OBJ_DEFMARKFUNC(lk_Tagmarkfunc_t);
#define LK_OBJ_DEFFREEFUNC(name) void name(lk_Object_t *self)
typedef LK_OBJ_DEFFREEFUNC(lk_Tagfreefunc_t);
struct lk_Tag {
    int                refc;
    lk_Vm_t           *vm;
    size_t             size;
    lk_Tagallocfunc_t *allocfunc;
    lk_Tagmarkfunc_t  *markfunc;
    lk_Tagfreefunc_t  *freefunc;
};
struct lk_Objectgroup {
    lk_Object_t *first;
    lk_Object_t *last;
};
struct lk_Common {
    lk_Object_t            *parent;
    Sequence_t              *ancestors;
    set_t               *slots;
    struct lk_Tag          *tag;
    struct lk_Mark {
        lk_Object_t        *prev;
        lk_Object_t        *next;
        struct lk_Objectgroup *objgroup;
        uint8_t             isref;
    }                       mark;
};
#define LK_VM(v) ((v)->obj.tag->vm)

/* used by ext - can't be in ext.h due to bootstrapping issues */
#define LK_EXT_DEFINIT(name) void name(lk_Vm_t *vm)
typedef LK_EXT_DEFINIT(lk_Libraryinitfunc_t);

/* required primitives */
#include "string.h"
#include "frame.h"
#include "gc.h"
#include "instr.h"
#include "obj.h"
#include "error.h"

/* todo: figure out a way to move this before req primitives */
#define LK_LIBRARY_DEFINECFUNCTION(name) static void name(lk_Object_t *self, lk_Frame_t *env)
typedef void lk_Cfuncfunc_t(lk_Object_t *self, lk_Frame_t *env);

/* actual def - add header to above #include's on lk_Vm_t change */
struct lk_Object {
    struct lk_Common obj;
};
struct lk_Vm {
    struct lk_Rsrcchain {
        uint8_t              isstring;
        lk_String_t         *rsrc;
        struct lk_Rsrcchain *prev;
    } *rsrc;
    struct lk_Rescue {
        jmp_buf              buf;
        struct lk_Rescue    *prev;
        struct lk_Rsrcchain *rsrc;
    } *rescue;
    lk_Instr_t *currinstr;
    lk_Frame_t *currentFrame;
    lk_Error_t *lasterror;
    lk_Gc_t *gc;
    lk_Frame_t *global;

    /* freq used primitive types */
    /* bool     */ lk_Object_t *t_nil, *t_bool, *t_true, *t_false,
                               *t_pi, *t_ni;
    /* char     */ lk_Object_t *t_char;
    /* cset     */ lk_Object_t *t_cset;
    /* error    */ lk_Object_t *t_error;
    /* file     */ lk_Object_t *t_file, *t_dir, *t_rf, *t_wf,
                               *t_stdin, *t_stdout, *t_stderr;
    /* vector   */ lk_Object_t *t_vector;
    /* fixnum   */ lk_Object_t *t_number, *t_int, *t_fi, *t_real, *t_fr;
    /* frame    */ lk_Object_t *t_frame;
    /* func     */ lk_Object_t *t_func, *t_sig, *t_kfunc, *t_cfunc, *t_gfunc;
    /* glist    */ lk_Object_t *t_glist;
    /* gset     */ lk_Object_t *t_gset;
    /* instr    */ lk_Object_t *t_instr;
    /* list     */ lk_Object_t *t_list;
    /* map      */ lk_Object_t *t_map;
    /* obj      */ lk_Object_t *t_obj;
    /* parser   */ lk_Object_t *t_parser, *t_prec;
    /* socket   */ lk_Object_t *t_socket;
    /* string   */ lk_Object_t *t_string;
    /* vm       */ lk_Object_t *t_vm;

    /* freq used strings */
    lk_String_t *str_type;
    lk_String_t *str_forward;
    lk_String_t *str_rescue;
    lk_String_t *str_onassign;
    lk_String_t *str_at;
    lk_String_t *str_filesep;

    /* statistics */
    struct {
        long totalInstructions;
        long totalFrames;
        long recycledFrames;
    } stat;
};

/* ext map */
LK_EXT_DEFINIT(lk_Vm_extinittypes);
LK_EXT_DEFINIT(lk_Vm_extinitfuncs);

/* new */
lk_Vm_t *lk_Vm_new(void);
void lk_Vm_free(lk_Vm_t *self);

/* eval */
lk_Frame_t *lk_Vm_evalfile(lk_Vm_t *self, const char *file, const char *base);
lk_Frame_t *lk_Vm_evalstring(lk_Vm_t *self, const char *code);
void lk_Vm_doevalfunc(lk_Vm_t *vm);
void lk_Vm_raisecstr(lk_Vm_t *self, const char *message,
                     ...) __attribute__((noreturn));
void lk_Vm_raiseerrno(lk_Vm_t *self) __attribute__((noreturn));
void lk_Vm_raiseerror(lk_Vm_t *self,
                      lk_Error_t *error) __attribute__((noreturn));
void lk_Vm_exit(lk_Vm_t *self) __attribute__((noreturn));
void lk_Vm_abort(lk_Vm_t *self, lk_Error_t *error) __attribute__((noreturn));
#endif
