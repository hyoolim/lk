#ifndef LK_INSTR_H
#define LK_INSTR_H

/* type */
typedef struct lk_Instr lk_Instr_t;
#include "vm.h"
#include "parser.h"
struct lk_Instr {
    struct lk_Common  obj;
    enum lk_Instrtype_t {
        LK_INSTRTYPE_FUNC = 1,
        LK_INSTRTYPE_APPLY,
        LK_INSTRTYPE_LIST,
        LK_INSTRTYPE_GROUP,
        LK_INSTRTYPE_STRING,
        LK_INSTRTYPE_CHAR,
        LK_INSTRTYPE_FIXINT,
        LK_INSTRTYPE_FIXF,
        LK_INSTRTYPE_APPLYMSG,
        LK_INSTRTYPE_FRAMEMSG,
        LK_INSTRTYPE_SELFMSG,
        LK_INSTRTYPE_MORE
    }                 type;
    lk_Instr_t       *prev;
    lk_Instr_t       *next;
    lk_String_t      *rsrc;
    int               line;
    int               column;
    lk_Object_t      *v;
    lk_String_t      *comment;
    uint8_t           opts;
};
#define LK_INSTR(v) ((lk_Instr_t *)(v))
#define LK_INSTROHASMSGARGS (1 << 0)
#define LK_INSTROEND        (1 << 1)

/* ext map */
LK_EXT_DEFINIT(lk_Instr_extinittypes);
LK_EXT_DEFINIT(lk_Instr_extinitfuncs);

/* new */
lk_Instr_t *lk_Instr_new(lk_Parser_t *parser);
lk_Instr_t *lk_Instr_newmore(lk_Parser_t *parser);
lk_Instr_t *lk_Instr_newfunc(lk_Parser_t *parser, lk_Instr_t *first);
lk_Instr_t *lk_Instr_newarglist(lk_Parser_t *parser, lk_Instr_t *first);
lk_Instr_t *lk_Instr_newcomment(lk_Parser_t *parser, lk_String_t *s);
lk_Instr_t *lk_Instr_newstring(lk_Parser_t *parser, lk_String_t *s);
lk_Instr_t *lk_Instr_newempty(lk_Parser_t *parser);
lk_Instr_t *lk_Instr_newfi(lk_Parser_t *parser, int i);
lk_Instr_t *lk_Instr_newchar(lk_Parser_t *parser, uint32_t c);
lk_Instr_t *lk_Instr_newff(lk_Parser_t *parser, double f);
lk_Instr_t *lk_Instr_newmessage(lk_Parser_t *parser, lk_String_t *name);
lk_Instr_t *lk_Instr_newframemessage(lk_Parser_t *parser, lk_String_t *name);

/* info */
void lk_Instr_print(lk_Instr_t *);
#endif
