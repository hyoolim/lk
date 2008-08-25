#ifndef LK_TYPES_H
#define LK_TYPES_H
#include "base/common.h"
#include "base/darray.h"
#include "base/number.h"
#include "base/qphash.h"

/* foward decl of all types */
typedef struct lk_object lk_bool_t;
typedef struct lk_char lk_char_t;
typedef struct lk_charset lk_charset_t;
typedef struct lk_env lk_env_t;
typedef struct lk_error lk_error_t;
typedef struct lk_file lk_file_t;
typedef struct lk_folder lk_folder_t;
typedef struct lk_func lk_func_t;
typedef struct lk_cfunc lk_cfunc_t;
typedef struct lk_gfunc lk_gfunc_t;
typedef struct lk_kfunc lk_kfunc_t;
typedef struct lk_sig lk_sig_t;
typedef struct lk_gc lk_gc_t;
typedef struct lk_instr lk_instr_t;
typedef struct lk_library lk_library_t;
typedef struct lk_seq lk_list_t;
typedef struct lk_map lk_map_t;
typedef struct lk_number lk_number_t;
typedef struct lk_object lk_object_t;
typedef struct lk_parser lk_parser_t;
typedef struct lk_prec lk_prec_t;
typedef struct lk_random lk_random_t;
typedef struct lk_scope lk_scope_t;
typedef struct lk_seq lk_seq_t;
typedef struct lk_socket lk_socket_t;
typedef struct lk_ipaddr lk_ipaddr_t;
typedef struct lk_seq lk_string_t;
typedef struct lk_vector lk_vector_t;
typedef struct lk_vm lk_vm_t;

/* common data for all lk objs */
typedef void lk_tagallocfunc_t(lk_object_t *self, lk_object_t *parent);
#define LK_OBJ_DEFMARKFUNC(name) void name(lk_object_t *self, void (*mark)(lk_object_t *self))
typedef LK_OBJ_DEFMARKFUNC(lk_tagmarkfunc_t);
typedef void lk_tagfreefunc_t(lk_object_t *self);
struct lk_tag {
    int                refc;
    lk_vm_t           *vm;
    size_t             size;
    lk_tagallocfunc_t *allocfunc;
    lk_tagmarkfunc_t  *markfunc;
    lk_tagfreefunc_t  *freefunc;
};
struct lk_objGroup {
    lk_object_t *first;
    lk_object_t *last;
};
struct lk_common {
    lk_object_t            *parent;
    darray_t              *ancestors;
    qphash_t               *slots;
    struct lk_tag          *tag;
    struct {
        lk_object_t        *prev;
        lk_object_t        *next;
        struct lk_objGroup *objgroup;
        uint8_t             isref;
    }                       mark;
};

/* used by ext - can't be in ext.h due to bootstrapping issues */
typedef void lk_libraryinitfunc_t(lk_vm_t *vm);
typedef void lk_cfunc_lk_t(lk_object_t *self, lk_scope_t *local);
typedef lk_object_t *lk_cfunc_r0_t(lk_object_t *self);
typedef lk_object_t *lk_cfunc_r1_t(lk_object_t *self, lk_object_t *a0type);
typedef lk_object_t *lk_cfunc_r2_t(lk_object_t *self, lk_object_t *a0type, lk_object_t *a1type);
typedef lk_object_t *lk_cfunc_r3_t(lk_object_t *self, lk_object_t *a0type, lk_object_t *a1type, lk_object_t *a2type);
typedef void lk_cfunc_v0_t(lk_object_t *self);
typedef void lk_cfunc_v1_t(lk_object_t *self, lk_object_t *a0type);
typedef void lk_cfunc_v2_t(lk_object_t *self, lk_object_t *a0type, lk_object_t *a1type);
typedef void lk_cfunc_v3_t(lk_object_t *self, lk_object_t *a0type, lk_object_t *a1type, lk_object_t *a2type);
typedef void lk_cfuncfunc_t(lk_object_t *self, lk_scope_t *local);

#define LK_BOOL(object) ((lk_bool_t *)(object))
#define LK_CHAR(object) ((lk_char_t *)(object))
#define LK_CHARSET(object) ((lk_charset_t *)(object))
#define LK_ENV(object) ((lk_env_t *)(object))
#define LK_ERROR(object) ((lk_error_t *)(object))
#define LK_FILE(object) ((lk_file_t *)(object))
#define LK_FOLDER(object) ((lk_folder_t *)(object))
#define LK_FUNC(object) ((lk_func_t *)(object))
#define LK_FUNCORUNNING  (1 << 0)
#define LK_FUNCOASSIGNED (1 << 1)
#define LK_CFUNC(object) ((lk_cfunc_t *)(object))
#define LK_GFUNC(object) ((lk_gfunc_t *)(object))
#define LK_KFUNC(object) ((lk_kfunc_t *)(object))
#define LK_SIG(object) ((lk_sig_t *)(object))
#define LK_GC(object) ((lk_gc_t *)(object))
#define LK_INSTR(object) ((lk_instr_t *)(object))
#define LK_INSTROHASMSGARGS (1 << 0)
#define LK_INSTROEND        (1 << 1)
#define LK_EXT(object) ((lk_library_t *)(object))
#define LK_LIST(object) ((lk_list_t *)(object))
#define LK_DARRAY(object) ((lk_list_t *)(object))
#define LK_MAP(object) ((lk_map_t *)(object))
#define LK_NUMBER(object) ((lk_number_t *)(object))
#define LK_OBJ(object) ((lk_object_t *)(object))
#define LK_PARSER(object) ((lk_parser_t *)(object))
#define LK_PREC(object) ((lk_prec_t *)(object))
#define LK_RANDOM(object) ((lk_random_t *)(object))
#define LK_RANDOM_N 624
#define LK_SCOPE(object) ((lk_scope_t *)(object))
#define LK_SEQ(object) ((lk_seq_t *)(object))
#define LK_SOCKET(object) ((lk_socket_t *)(object))
#define LK_IPADDR(object) ((lk_ipaddr_t *)(object))
#define LK_STRING(object) ((lk_string_t *)(object))
#define LK_VECTOR(object) ((lk_vector_t *)(object))
#define LK_VM(object) ((object)->o.tag->vm)
#endif
