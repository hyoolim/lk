#ifndef LK_INSTR_H
#define LK_INSTR_H
#include "types.h"

/* type */
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
    lk_str_t      *rsrc;
    int               line;
    int               column;
    lk_obj_t      *v;
    lk_str_t      *comment;
    uint8_t           opts;
};

/* ext map */
void lk_instr_typeinit(lk_vm_t *vm);
void lk_instr_libinit(lk_vm_t *vm);

/* new */
lk_instr_t *lk_instr_new(lk_parser_t *parser);
lk_instr_t *lk_instr_newmore(lk_parser_t *parser);
lk_instr_t *lk_instr_newfunc(lk_parser_t *parser, lk_instr_t *first);
lk_instr_t *lk_instr_newarglist(lk_parser_t *parser, lk_instr_t *first);
lk_instr_t *lk_instr_newcomment(lk_parser_t *parser, lk_str_t *s);
lk_instr_t *lk_instr_newstr(lk_parser_t *parser, lk_str_t *s);
lk_instr_t *lk_instr_newempty(lk_parser_t *parser);
lk_instr_t *lk_instr_newNumber(lk_parser_t *parser, double num);
lk_instr_t *lk_instr_newchar(lk_parser_t *parser, uint32_t c);
lk_instr_t *lk_instr_newmessage(lk_parser_t *parser, lk_str_t *name);
lk_instr_t *lk_instr_newscopemessage(lk_parser_t *parser, lk_str_t *name);

/* info */
void lk_instr_print(lk_instr_t *);
lk_num_t *lk_instr_column(lk_obj_t *self);
lk_num_t *lk_instr_line(lk_obj_t *self);
lk_str_t *lk_instr_message(lk_obj_t *self);
lk_str_t *lk_instr_resource(lk_obj_t *self);
#endif
