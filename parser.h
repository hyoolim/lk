#ifndef LK_PARSER_H
#define LK_PARSER_H

/* type */
typedef struct lk_parser lk_parser_t;
typedef struct lk_prec lk_prec_t;
#define LK_PARSER(v) ((lk_parser_t *)(v))
#include "vm.h"
#include "instr.h"
struct lk_parser {
    struct lk_common o;
    const lk_string_t *text;
    const char        *error;
    qphash_t          *binaryops;
    qphash_t          *precs;
    darray_t         *tokentypes;
    darray_t         *tokenvalues;
    darray_t         *words;
    darray_t         *ops;
    darray_t         *comments;
    int                textpos;
    int                line;
    int                column;
    int                opsize;
    int                isterminated;
};
struct lk_prec {
    struct lk_common o;
    int              level;
    enum lk_precassoc_t {
        LK_PREC_ASSOC_LEFT = 1,
        LK_PREC_ASSOC_RIGHT,
        LK_PREC_ASSOC_NON
    }                assoc;
};
#define LK_PREC(v) ((lk_prec_t *)(v))

/* ext map */
void lk_parser_libPreInit(lk_vm_t *vm);
void lk_parser_libInit(lk_vm_t *vm);

/* new */
lk_parser_t *lk_parser_new(lk_vm_t *vm);

/* eval */
lk_instr_t *lk_parser_parse(lk_parser_t *self, const lk_string_t *text);
lk_instr_t *lk_parser_getmore(lk_parser_t *self);
#endif
