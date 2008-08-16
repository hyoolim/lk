#ifndef LK_PARSER_H
#define LK_PARSER_H

/* type */
typedef struct lk_Parser lk_Parser_t;
typedef struct lk_Prec lk_Prec_t;
#include "vm.h"
#include "instr.h"
struct lk_Parser {
    struct lk_Common   obj;
    const lk_String_t *text;
    const char        *error;
    set_t          *binaryops;
    set_t          *precs;
    Sequence_t         *tokentypes;
    Sequence_t         *tokenvalues;
    Sequence_t         *words;
    Sequence_t         *ops;
    Sequence_t         *comments;
    int                textpos;
    int                line;
    int                column;
    int                opcount;
    int                isterminated;
};
#define LK_PARSER(v) ((lk_Parser_t *)(v))
struct lk_Prec {
    struct lk_Common obj;
    int              level;
    enum lk_Precassoc_t {
        LK_PREC_ASSOC_LEFT = 1,
        LK_PREC_ASSOC_RIGHT,
        LK_PREC_ASSOC_NON
    }                assoc;
};
#define LK_PREC(v) ((lk_Prec_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Parser_extinittypes);
LK_EXT_DEFINIT(lk_Parser_extinitfuncs);

/* new */
lk_Parser_t *lk_Parser_new(lk_Vm_t *vm);

/* eval */
lk_Instr_t *lk_Parser_parse(lk_Parser_t *self, const lk_String_t *text);
lk_Instr_t *lk_Parser_getmore(lk_Parser_t *self);
#endif
