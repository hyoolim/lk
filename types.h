#ifndef LK_TYPES_H
#define LK_TYPES_H
#include "base/common.h"
#include "base/darray.h"
#include "base/num.h"
#include "base/qphash.h"

/* foward decl of all types */
typedef struct lk_obj lk_bool_t;
typedef struct lk_char lk_char_t;
typedef struct lk_charset lk_charset_t;
typedef struct lk_env lk_env_t;
typedef struct lk_err lk_err_t;
typedef struct lk_file lk_file_t;
typedef struct lk_dir lk_dir_t;
typedef struct lk_func lk_func_t;
typedef struct lk_cfunc lk_cfunc_t;
typedef struct lk_gfunc lk_gfunc_t;
typedef struct lk_kfunc lk_kfunc_t;
typedef struct lk_sig lk_sig_t;
typedef struct lk_gc lk_gc_t;
typedef struct lk_instr lk_instr_t;
typedef struct lk_dl lk_dl_t;
typedef struct lk_seq lk_list_t;
typedef struct lk_map lk_map_t;
typedef struct lk_num lk_num_t;
typedef struct lk_obj lk_obj_t;
typedef struct lk_parser lk_parser_t;
typedef struct lk_prec lk_prec_t;
typedef struct lk_pipe lk_pipe_t;
typedef struct lk_rand lk_rand_t;
typedef struct lk_scope lk_scope_t;
typedef struct lk_seq lk_seq_t;
typedef struct lk_socket lk_socket_t;
typedef struct lk_ipaddr lk_ipaddr_t;
typedef struct lk_seq lk_str_t;
typedef struct lk_vec lk_vec_t;
typedef struct lk_vm lk_vm_t;

/* common data for all lk objs */
typedef void lk_tagallocfunc_t(lk_obj_t *self, lk_obj_t *parent);
#define LK_OBJ_DEFMARKFUNC(name) void name(lk_obj_t *self, void (*mark)(lk_obj_t *self))
typedef LK_OBJ_DEFMARKFUNC(lk_tagmarkfunc_t);
typedef void lk_tagfreefunc_t(lk_obj_t *self);
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
    lk_obj_t            *parent;
    darray_t              *ancestors;
    qphash_t               *slots;
    struct lk_tag          *tag;
    struct {
        lk_obj_t        *prev;
        lk_obj_t        *next;
        struct lk_objgroup *objgroup;
        uint8_t             isref;
    }                       mark;
};

/* used by ext - can't be in lib.h due to bootstrapping issues */
typedef void lk_libraryinitfunc_t(lk_vm_t *vm);
typedef void lk_cfunc_lk_t(lk_obj_t *self, lk_scope_t *local);
typedef lk_obj_t *lk_cfunc_r0_t(lk_obj_t *self);
typedef lk_obj_t *lk_cfunc_r1_t(lk_obj_t *self, lk_obj_t *a0type);
typedef lk_obj_t *lk_cfunc_r2_t(lk_obj_t *self, lk_obj_t *a0type, lk_obj_t *a1type);
typedef lk_obj_t *lk_cfunc_r3_t(lk_obj_t *self, lk_obj_t *a0type, lk_obj_t *a1type, lk_obj_t *a2type);
typedef void lk_cfunc_v0_t(lk_obj_t *self);
typedef void lk_cfunc_v1_t(lk_obj_t *self, lk_obj_t *a0type);
typedef void lk_cfunc_v2_t(lk_obj_t *self, lk_obj_t *a0type, lk_obj_t *a1type);
typedef void lk_cfunc_v3_t(lk_obj_t *self, lk_obj_t *a0type, lk_obj_t *a1type, lk_obj_t *a2type);
typedef void lk_cfuncfunc_t(lk_obj_t *self, lk_scope_t *local);

#define LK_BOOL(obj) ((lk_bool_t *)(obj))
#define LK_CHAR(obj) ((lk_char_t *)(obj))
#define LK_CHARSET(obj) ((lk_charset_t *)(obj))
#define LK_ENV(obj) ((lk_env_t *)(obj))
#define LK_ERROR(obj) ((lk_err_t *)(obj))
#define LK_FILE(obj) ((lk_file_t *)(obj))
#define LK_DIR(obj) ((lk_dir_t *)(obj))
#define LK_FUNC(obj) ((lk_func_t *)(obj))
#define LK_FUNCORUNNING  (1 << 0)
#define LK_FUNCOASSIGNED (1 << 1)
#define LK_CFUNC(obj) ((lk_cfunc_t *)(obj))
#define LK_GFUNC(obj) ((lk_gfunc_t *)(obj))
#define LK_KFUNC(obj) ((lk_kfunc_t *)(obj))
#define LK_SIG(obj) ((lk_sig_t *)(obj))
#define LK_GC(obj) ((lk_gc_t *)(obj))
#define LK_INSTR(obj) ((lk_instr_t *)(obj))
#define LK_INSTROHASMSGARGS (1 << 0)
#define LK_INSTROEND        (1 << 1)
#define LK_DL(obj) ((lk_dl_t *)(obj))
#define LK_LIST(obj) ((lk_list_t *)(obj))
#define LK_DARRAY(obj) ((lk_list_t *)(obj))
#define LK_MAP(obj) ((lk_map_t *)(obj))
#define LK_NUMBER(obj) ((lk_num_t *)(obj))
#define LK_OBJ(obj) ((lk_obj_t *)(obj))
#define LK_PARSER(obj) ((lk_parser_t *)(obj))
#define LK_PREC(obj) ((lk_prec_t *)(obj))
#define LK_PIPE(obj) ((lk_pipe_t *)(obj))
#define LK_RANDOM(obj) ((lk_rand_t *)(obj))
#define LK_RANDOM_N 624
#define LK_SCOPE(obj) ((lk_scope_t *)(obj))
#define LK_SEQ(obj) ((lk_seq_t *)(obj))
#define LK_SOCKET(obj) ((lk_socket_t *)(obj))
#define LK_IPADDR(obj) ((lk_ipaddr_t *)(obj))
#define LK_STRING(obj) ((lk_str_t *)(obj))
#define LK_VECTOR(obj) ((lk_vec_t *)(obj))
#define LK_VM(obj) ((obj)->o.tag->vm)
#endif
