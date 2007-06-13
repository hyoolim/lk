#ifndef LK_PARSER_H
#define LK_PARSER_H

/* type */
typedef struct lk_parser lk_parser_t;
typedef struct lk_prec lk_prec_t;
#include "vm.h"
#include "instr.h"
struct lk_parser {
    struct lk_common   co;
    const lk_string_t *text;
    const char        *error;
    pt_set_t          *unaryops;
    pt_set_t          *binaryops;
    pt_set_t          *precs;
    pt_list_t         *tokentypes;
    pt_list_t         *tokenvalues;
    pt_list_t         *words;
    pt_list_t         *ops;
    pt_list_t         *comments;
    int                textpos;
    int                line;
    int                column;
    int                opcount;
    int                isterminated;
};
#define LK_PARSER(v) ((lk_parser_t *)(v))
struct lk_prec {
    struct lk_common co;
    int              level;
    enum lk_precassoc_t {
        LK_PREC_ASSOC_LEFT = 1,
        LK_PREC_ASSOC_RIGHT,
        LK_PREC_ASSOC_NON
    }                assoc;
};
#define LK_PREC(v) ((lk_prec_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_parser_extinittypes);
LK_EXT_DEFINIT(lk_parser_extinitfuncs);

/* new */
lk_parser_t *lk_parser_new(lk_vm_t *vm);

/* eval */
lk_instr_t *lk_parser_parse(lk_parser_t *self, const lk_string_t *text);
lk_instr_t *lk_parser_getmore(lk_parser_t *self);
#endif
