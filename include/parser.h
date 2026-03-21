#ifndef LK_PARSER_H
#define LK_PARSER_H
#include "types.h"

/* type */
struct lk_parser {
    struct lk_common o;
    const lk_str_t *text;
    const char        *err;
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

/* ext map */
void lk_parser_typeinit(lk_vm_t *vm);
void lk_parser_libinit(lk_vm_t *vm);

/* new */
lk_parser_t *lk_parser_new(lk_vm_t *vm);

/* eval */
lk_instr_t *lk_parser_parse(lk_parser_t *self, const lk_str_t *text);
lk_instr_t *lk_parser_getmore(lk_parser_t *self);
#endif
