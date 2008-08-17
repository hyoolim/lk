#ifndef LK_VM_H
#define LK_VM_H

/* generic lib for handling common types of data */
#include "base/common.h"
#include "base/array.h"
#include "base/number.h"
#include "base/set.h"
#include "base/string.h"
#include <setjmp.h>

/* type - see further down below for actual def */
typedef struct lk_vm lk_vm_t;
typedef struct lk_object lk_object_t;
#define LK_OBJ(v) ((lk_object_t *)(v))

/* common data for all lk objs */
#define LK_OBJ_DEFALLOCFUNC(name) void name(lk_object_t *self, lk_object_t *parent)
typedef LK_OBJ_DEFALLOCFUNC(lk_tagallocfunc_t);
#define LK_OBJ_DEFMARKFUNC(name) void name(lk_object_t *self, void (*mark)(lk_object_t *self))
typedef LK_OBJ_DEFMARKFUNC(lk_tagmarkfunc_t);
#define LK_OBJ_DEFFREEFUNC(name) void name(lk_object_t *self)
typedef LK_OBJ_DEFFREEFUNC(lk_tagfreefunc_t);
struct lk_tag {
    int                refc;
    lk_vm_t           *vm;
    size_t             size;
    lk_tagallocfunc_t *allocfunc;
    lk_tagmarkfunc_t  *markfunc;
    lk_tagfreefunc_t  *freefunc;
};
struct lk_objectgroup {
    lk_object_t *first;
    lk_object_t *last;
};
struct lk_common {
    lk_object_t            *parent;
    array_t              *ancestors;
    set_t               *slots;
    struct lk_tag          *tag;
    struct lk_mark {
        lk_object_t        *prev;
        lk_object_t        *next;
        struct lk_objectgroup *objgroup;
        uint8_t             isref;
    }                       mark;
};
#define LK_VM(v) ((v)->obj.tag->vm)

/* used by ext - can't be in ext.h due to bootstrapping issues */
#define LK_EXT_DEFINIT(name) void name(lk_vm_t *vm)
typedef LK_EXT_DEFINIT(lk_libraryinitfunc_t);

/* required primitives */
#include "string.h"
#include "frame.h"
#include "gc.h"
#include "instr.h"
#include "obj.h"
#include "error.h"

/* todo: figure out a way to move this before req primitives */
#define LK_LIBRARY_DEFINECFUNCTION(name) static void name(lk_object_t *self, lk_frame_t *env)
typedef void lk_cfuncfunc_t(lk_object_t *self, lk_frame_t *env);

/* actual def - add header to above #include's on lk_vm_t change */
struct lk_object {
    struct lk_common obj;
};
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
    lk_frame_t *currentFrame;
    lk_error_t *lasterror;
    lk_gc_t *gc;
    lk_frame_t *global;

    /* freq used primitive types */
    /* bool     */ lk_object_t *t_nil, *t_bool, *t_true, *t_false,
                               *t_pi, *t_ni;
    /* char     */ lk_object_t *t_char;
    /* charset  */ lk_object_t *t_charset;
    /* error    */ lk_object_t *t_error;
    /* file     */ lk_object_t *t_file, *t_folder, *t_rf, *t_wf,
                               *t_stdin, *t_stdout, *t_stderr;
    /* vector   */ lk_object_t *t_vector;
    /* fixnum   */ lk_object_t *t_number, *t_int, *t_fi, *t_real, *t_fr;
    /* frame    */ lk_object_t *t_frame;
    /* func     */ lk_object_t *t_func, *t_sig, *t_kfunc, *t_cfunc, *t_gfunc;
    /* seq      */ lk_object_t *t_seq;
    /* gset     */ lk_object_t *t_gset;
    /* instr    */ lk_object_t *t_instr;
    /* list     */ lk_object_t *t_list;
    /* map      */ lk_object_t *t_map;
    /* obj      */ lk_object_t *t_obj;
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
        long totalFrames;
        long recycledFrames;
    } stat;
};

/* ext map */
LK_EXT_DEFINIT(lk_vm_extinittypes);
LK_EXT_DEFINIT(lk_vm_extinitfuncs);

/* new */
lk_vm_t *lk_vm_new(void);
void lk_vm_free(lk_vm_t *self);

/* eval */
lk_frame_t *lk_vm_evalfile(lk_vm_t *self, const char *file, const char *base);
lk_frame_t *lk_vm_evalstring(lk_vm_t *self, const char *code);
void lk_vm_doevalfunc(lk_vm_t *vm);
void lk_vm_raisecstr(lk_vm_t *self, const char *message,
                     ...) __attribute__((noreturn));
void lk_vm_raiseerrno(lk_vm_t *self) __attribute__((noreturn));
void lk_vm_raiseerror(lk_vm_t *self,
                      lk_error_t *error) __attribute__((noreturn));
void lk_vm_exit(lk_vm_t *self) __attribute__((noreturn));
void lk_vm_abort(lk_vm_t *self, lk_error_t *error) __attribute__((noreturn));
#endif
