#ifndef LK_INSTR_H
#define LK_INSTR_H

/* type */
typedef struct lk_instr lk_instr_t;
#define LK_INSTR(v) ((lk_instr_t *)(v))
#define LK_INSTROHASMSGARGS (1 << 0)
#define LK_INSTROEND        (1 << 1)
#include "vm.h"
#include "parser.h"
struct lk_instr {
    struct lk_common o;
    enum lk_instrtype_t {
        LK_INSTRTYPE_FUNC = 1,
        LK_INSTRTYPE_APPLY,
        LK_INSTRTYPE_LIST,
        LK_INSTRTYPE_GROUP,
        LK_INSTRTYPE_STRING,
        LK_INSTRTYPE_CHAR,
        LK_INSTRTYPE_NUMBER,
        LK_INSTRTYPE_APPLYMSG,
        LK_INSTRTYPE_SCOPEMSG,
        LK_INSTRTYPE_SELFMSG,
        LK_INSTRTYPE_MORE
    }                 type;
    lk_instr_t       *prev;
    lk_instr_t       *next;
    lk_string_t      *rsrc;
    int               line;
    int               column;
    lk_object_t      *v;
    lk_string_t      *comment;
    uint8_t           opts;
};

/* ext map */
LK_LIB_DEFINEINIT(lk_instr_libPreInit);
LK_LIB_DEFINEINIT(lk_instr_libInit);

/* new */
lk_instr_t *lk_instr_new(lk_parser_t *parser);
lk_instr_t *lk_instr_newmore(lk_parser_t *parser);
lk_instr_t *lk_instr_newfunc(lk_parser_t *parser, lk_instr_t *first);
lk_instr_t *lk_instr_newarglist(lk_parser_t *parser, lk_instr_t *first);
lk_instr_t *lk_instr_newcomment(lk_parser_t *parser, lk_string_t *s);
lk_instr_t *lk_instr_newstring(lk_parser_t *parser, lk_string_t *s);
lk_instr_t *lk_instr_newempty(lk_parser_t *parser);
lk_instr_t *lk_instr_newNumber(lk_parser_t *parser, double number);
lk_instr_t *lk_instr_newchar(lk_parser_t *parser, uint32_t c);
lk_instr_t *lk_instr_newmessage(lk_parser_t *parser, lk_string_t *name);
lk_instr_t *lk_instr_newscopemessage(lk_parser_t *parser, lk_string_t *name);

/* info */
void lk_instr_print(lk_instr_t *);
#endif
