#include "parser.h"
#include "ext.h"
#include "string.h"
#define PARSER (LK_PARSER(self))

/* ext map - types */
static void setunaryop(lk_parser_t *self, const char *op, const char *subst) {
    lk_vm_t *vm = LK_VM(self);
    *(lk_string_t **)set_set(self->unaryops, lk_string_newfromcstr(vm, op)) = lk_string_newfromcstr(vm, subst);
}
static void setbinaryop(lk_parser_t *self, const char *op, const char *subst) {
    lk_vm_t *vm = LK_VM(self);
    *(lk_string_t **)set_set(self->binaryops, lk_string_newfromcstr(vm, op)) = lk_string_newfromcstr(vm, subst);
}
static void setprec(lk_parser_t *self, const char *op, int level, enum lk_precassoc_t assoc) {
    lk_vm_t *vm = LK_VM(self);
    lk_prec_t *prec = LK_PREC(lk_object_alloc(vm->t_prec));
    prec->level = level;
    prec->assoc = assoc;
    *(lk_prec_t **)set_set(self->precs, lk_string_newfromcstr(vm, op)) = prec;
}
static LK_OBJECT_DEFALLOCFUNC(alloc__parser) {
    PARSER->unaryops = set_alloc(
    sizeof(lk_prec_t *), lk_object_hashcode, lk_object_keycmp);
    PARSER->binaryops = set_alloc(
    sizeof(lk_prec_t *), lk_object_hashcode, lk_object_keycmp);
    PARSER->precs = set_alloc(
    sizeof(lk_prec_t *), lk_object_hashcode, lk_object_keycmp);
    PARSER->tokentypes = list_allocptr();
    PARSER->tokenvalues = list_allocptr();
    PARSER->words = list_allocptr();
    PARSER->ops = list_allocptr();
    PARSER->comments = list_allocptr();
    /* unary op names */
    setunaryop(PARSER, "!",    "not?");
    setunaryop(PARSER, "@",    "to list");
    setunaryop(PARSER, "#",    "count");
    setunaryop(PARSER, "$",    "to string");
    setunaryop(PARSER, "-",    "negate");
    setunaryop(PARSER, "+",    "to number");
    /* binary op names */
    setbinaryop(PARSER, "~",   "to");
    setbinaryop(PARSER, "~=",  "match?");
    setbinaryop(PARSER, "!=",  "ne?");
    setbinaryop(PARSER, "@",   "at");
    setbinaryop(PARSER, "@=",  "set!");
    setbinaryop(PARSER, "%",   "divide");
    setbinaryop(PARSER, "%=",  "divide!");
    setbinaryop(PARSER, "%%",  "modulo");
    setbinaryop(PARSER, "%%=", "modulo!");
    setbinaryop(PARSER, "&&",  "and");
    setbinaryop(PARSER, "*",   "multiply");
    setbinaryop(PARSER, "*=",  "multiply!");
    setbinaryop(PARSER, "**",  "repeat");
    setbinaryop(PARSER, "**=", "repeat!");
    setbinaryop(PARSER, "-",   "subtract");
    setbinaryop(PARSER, "-=",  "subtract!");
    setbinaryop(PARSER, "$",   "to string");
    setbinaryop(PARSER, "->",  "send");
    setbinaryop(PARSER, "=",   ".assign!");
    setbinaryop(PARSER, "==",  "eq?");
    setbinaryop(PARSER, "+",   "add");
    setbinaryop(PARSER, "+=",  "add!");
    setbinaryop(PARSER, "++",  "concat");
    setbinaryop(PARSER, "++=", "concat!");
    setbinaryop(PARSER, ":",   ".define!");
    setbinaryop(PARSER, ":=",  ".define_assign!");
    setbinaryop(PARSER, "!!",  "else");
    setbinaryop(PARSER, "||",  "or");
    setbinaryop(PARSER, "|||", "nil_or");
    setbinaryop(PARSER, "<",   "lt?");
    setbinaryop(PARSER, "<=",  "le?");
    setbinaryop(PARSER, "<=>", "cmp");
    setbinaryop(PARSER, "<<",  "concat!");
    setbinaryop(PARSER, ">",   "gt?");
    setbinaryop(PARSER, ">=",  "ge?");
    setbinaryop(PARSER, "/",   "send");
    setbinaryop(PARSER, "??",  "then");
    /* prec map */
    setprec(PARSER, "@",   100000, LK_PREC_ASSOC_LEFT); /* misc */
    setprec(PARSER, "/",    90000, LK_PREC_ASSOC_LEFT);
    setprec(PARSER, ":",    80000, LK_PREC_ASSOC_LEFT);
    setprec(PARSER, "*",    70000, LK_PREC_ASSOC_LEFT); /* arith */
    setprec(PARSER, "**",   70000, LK_PREC_ASSOC_LEFT);
    setprec(PARSER, "%",    70000, LK_PREC_ASSOC_LEFT);
    setprec(PARSER, "%%",   70000, LK_PREC_ASSOC_LEFT);
    setprec(PARSER, "+",    60000, LK_PREC_ASSOC_LEFT); /* add/concat/sub */
    setprec(PARSER, "++",   60000, LK_PREC_ASSOC_LEFT);
    setprec(PARSER, "-",    60000, LK_PREC_ASSOC_LEFT);
    setprec(PARSER, "$",    55000, LK_PREC_ASSOC_LEFT);
    setprec(PARSER, "==",   50000, LK_PREC_ASSOC_NON); /* comparison */
    setprec(PARSER, "!=",   50000, LK_PREC_ASSOC_NON);
    setprec(PARSER, "~=",   50000, LK_PREC_ASSOC_NON);
    setprec(PARSER, "<",    50000, LK_PREC_ASSOC_NON);
    setprec(PARSER, ">",    50000, LK_PREC_ASSOC_NON);
    setprec(PARSER, "<=",   50000, LK_PREC_ASSOC_NON);
    setprec(PARSER, ">=",   50000, LK_PREC_ASSOC_NON);
    setprec(PARSER, "<=>",  50000, LK_PREC_ASSOC_NON);
    setprec(PARSER, "&&",   40000, LK_PREC_ASSOC_LEFT); /* logical */
    setprec(PARSER, "||",   40000, LK_PREC_ASSOC_LEFT);
    setprec(PARSER, "|||",  40000, LK_PREC_ASSOC_LEFT);
    setprec(PARSER, "??",   20000, LK_PREC_ASSOC_RIGHT); /* flow */
    setprec(PARSER, "!!",   19999, LK_PREC_ASSOC_RIGHT);
    setprec(PARSER, "->",  -10000, LK_PREC_ASSOC_LEFT); /* misc - low prec */
}
static LK_OBJECT_DEFMARKFUNC(mark__parser) {
    mark(LK_O(PARSER->text));
    if(PARSER->tokenvalues != NULL
    ) LIST_EACHPTR(PARSER->tokenvalues, i, v, mark(v));
    if(PARSER->unaryops != NULL) SET_EACH(PARSER->unaryops, item,
        mark(LK_O(item->key));
        mark(SETITEM_VALUE(lk_object_t *, item));
    );
    if(PARSER->binaryops != NULL) SET_EACH(PARSER->binaryops, item,
        mark(LK_O(item->key));
        mark(SETITEM_VALUE(lk_object_t *, item));
    );
    if(PARSER->precs != NULL) SET_EACH(PARSER->precs, item,
        mark(LK_O(item->key));
        mark(SETITEM_VALUE(lk_object_t *, item));
    );
}
static LK_OBJECT_DEFFREEFUNC(free__parser) {
    if(PARSER->precs != NULL) set_free(PARSER->precs);
    if(PARSER->tokentypes != NULL) list_free(PARSER->tokentypes);
    if(PARSER->tokenvalues != NULL) list_free(PARSER->tokenvalues);
    if(PARSER->words != NULL) list_free(PARSER->words);
    if(PARSER->ops != NULL) list_free(PARSER->ops);
}
LK_EXT_DEFINIT(lk_parser_extinittypes) {
    lk_object_t *obj = vm->t_object;
    vm->t_prec = lk_object_allocwithsize(obj, sizeof(lk_prec_t));
    vm->t_parser = lk_object_allocwithsize(obj, sizeof(lk_parser_t));
    lk_object_setallocfunc(vm->t_parser, alloc__parser);
    lk_object_setmarkfunc(vm->t_parser, mark__parser);
    lk_object_setfreefunc(vm->t_parser, free__parser);
}

/* ext map - funcs */
LK_EXT_DEFINIT(lk_parser_extinitfuncs) {
    lk_ext_set(vm->t_vm, "Precedence", vm->t_prec);
    lk_ext_set(vm->t_vm, "Parser", vm->t_parser);
}

/* new */
lk_parser_t *lk_parser_new(lk_vm_t *vm) {
    return LK_PARSER(lk_object_alloc(vm->t_parser));
}

/* eval */
typedef enum tokentype_t {
    NONE = 0, TERMINATOR,
    WORD, OP, CHAR, NUMBER, STRING,
    COMMENT, WS2,
    FUNC_BEGIN, FUNC_SEP, FUNC_END,
    LIST_BEGIN, LIST_END,
    GROUP_BEGIN, GROUP_END
} tokentype_t;

/* parse helpers */
/* helper func proto */
#define READTOKENFUNC(name) tokentype_t name(lk_parser_t *self)
#define READFUNC(name) int name(lk_parser_t *self)
typedef READFUNC(readfunc_t);
/* common calc on text */
#define CHARAT(self, at) (list_getuchar(LIST((self)->text), (at)))
#define CHARCURR(self) (CHARAT((self), (self)->textpos))
#define CHARNEXT(self) (CHARAT((self), ++ (self)->textpos))
#define CHARPREV(self) (CHARAT((self), -- (self)->textpos))
#define CHARIN(c, s) (strchr((s), (c)))
#define ISINSIDE(self) (0 <= (self)->textpos \
&& (self)->textpos < LIST_COUNT(LIST((self)->text)))
#define CHARCURRIN(self, s) (CHARIN(CHARCURR(self), (s)))
#define ISCURR(self, s) (ISINSIDE(self) && CHARCURRIN((self), (s)))
#define ISNOTCURR(self, s) (ISINSIDE(self) && !CHARCURRIN((self), (s)))
#define CHARNEXTWHILE(self, s) while(ISCURR((self), (s))) CHARNEXT(self)
#define CHARNEXTUNTIL(self, s) while(ISNOTCURR((self), (s))) CHARNEXT(self)
/* chars */
#define UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER "abcdefghijklmnopqrstuvwxyz"
#define ALPHA "." UPPER LOWER
#define DIGIT "0123456789"
#define ALNUM ALPHA DIGIT
#define LETTER "_" ALNUM
#define NEWLINE "\n\r"
#define SPACE " \t\f\v"
#define WS SPACE NEWLINE
#define LISTSEP ";"
#define EXPRSEP ";"
/* instr type conversion */
#define FRAME2APPLY(type) ((type) - LK_INSTRTYPE_FRAMEMSG + LK_INSTRTYPE_APPLYMSG)

/* */
static lk_string_t *getunaryop(lk_parser_t *self, lk_string_t *op) {
    setitem_t *item = set_get(self->unaryops, op);
    return item != NULL ? SETITEM_VALUE(lk_string_t *, item) : op;
}
static lk_string_t *getbinaryop(lk_parser_t *self, lk_string_t *op) {
    setitem_t *item = set_get(self->binaryops, op);
    return item != NULL ? SETITEM_VALUE(lk_string_t *, item) : op;
}
static lk_prec_t *getprec(lk_parser_t *self, lk_instr_t *op) {
    setitem_t *item = set_get(self->precs, op->v);
    return item != NULL ? SETITEM_VALUE(lk_prec_t *, item) : NULL;
}
static lk_prec_t *shiftreduce(lk_parser_t *self, lk_instr_t *op) {
    lk_prec_t *curr = op != NULL ? getprec(self, op) : NULL;
    int curr_level, prev_level;
    enum lk_precassoc_t assoc;
    /* reduce - NULL reduces everything */
    while(LIST_COUNT(self->ops) > self->opcount) {
        int is_break = 1;
        lk_instr_t *top = list_peekptr(self->ops);
        list_t *topstr = LIST(top->v);
        lk_prec_t *prev = getprec(self, top);
        if(op == NULL) is_break = 0;
        else {
            if(curr != NULL) curr_level = curr->level;
            else {
                if(list_getuchar(topstr, -1) == '=') curr_level = 10000;
                else curr_level = 30000;
            }
            if(prev != NULL) {
                prev_level = prev->level;
                assoc = prev->assoc;
            } else {
                if(list_getuchar(topstr, -1) == '=') {
                    prev_level = 10000;
                    assoc = LK_PREC_ASSOC_RIGHT;
                } else {
                    prev_level = 30000;
                    assoc = LK_PREC_ASSOC_LEFT;
                }
            }
            if(prev_level > curr_level) is_break = 0;
            else if(prev_level == curr_level) {
                if(assoc == LK_PREC_ASSOC_LEFT) is_break = 0;
                else if(assoc == LK_PREC_ASSOC_NON) {
                    lk_vm_raisecstr(VM,
                    "Cannot chain non-associative operators");
                }
            }
        }
        if(is_break) break;
        else {
            lk_instr_t *arg = list_popptr(self->words);
            lk_instr_t *last = list_peekptr(self->words);
            list_popptr(self->ops);
            top->v = LK_O(getbinaryop(self, LK_STRING(top->v)));
            topstr = LIST(top->v);
            /* rec / arg -> rec /arg */
            if((arg->type == LK_INSTRTYPE_FRAMEMSG
            || arg->type == LK_INSTRTYPE_STRING)
            && list_cmpcstr(topstr, "send") == 0) {
                top = arg;
                top->type = LK_INSTRTYPE_APPLYMSG;
                goto gotarg;
            /* rec ?? arg -> rec ?? { arg } */
            } else if(arg->type != LK_INSTRTYPE_FUNC
                   && list_cmpcstr(topstr, "then") == 0) {
                arg = lk_instr_newfunc(self, arg);
            }
            arg = lk_instr_newarglist(self, arg);
            (top->next = arg)->prev = top;
            gotarg:
            while(last->next != NULL) last = last->next;
            (last->next = top)->prev = last;
        }
    }
    /* shift */
    if(op != NULL) list_pushptr(self->ops, op);
    return curr;
}

static const tokentype_t miscmap[] = {
    /*   0 */ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    /*   8 */ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    /*  16 */ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    /*  24 */ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    /*  32 */ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    /*  40 */ GROUP_BEGIN, GROUP_END, NONE, NONE, NONE, NONE, NONE, NONE,
    /*  48 */ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    /*  56 */ NONE, NONE, NONE, TERMINATOR, NONE, NONE, NONE, NONE,
    /*  64 */ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    /*  72 */ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    /*  80 */ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    /*  88 */ NONE, NONE, NONE, LIST_BEGIN, NONE, LIST_END, NONE, NONE,
    /*  96 */ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    /* 104 */ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    /* 112 */ NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    /* 120 */ NONE, NONE, NONE, FUNC_BEGIN, NONE, FUNC_END, NONE, NONE
};
static READTOKENFUNC(readtoken_misc) {
    tokentype_t type = miscmap[CHARCURR(self)];
    if(type == NONE) return NONE;
    else {
        CHARNEXT(self);
        return type;
    }
}

/* whitespace */
static void incline(lk_parser_t *self) {
    if(CHARCURR(self) == '\r') {
        if(CHARNEXT(self) != '\n') CHARPREV(self);
        self->line ++;
    } else if(CHARCURR(self) == '\n') {
        self->line ++;
    }
}
static READTOKENFUNC(readtoken_ws) {
    if(ISCURR(self, SPACE)) {
        CHARNEXT(self);
        for(; ISINSIDE(self); CHARNEXT(self)) {
            if(strchr(SPACE, CHARCURR(self))) continue;
            if(strchr(NEWLINE, CHARCURR(self))) {
                incline(self);
                self->isterminated = 1;
                continue;
            }
            break;
        }
        return WS2;
    } else if(ISCURR(self, NEWLINE)) {
        incline(self);
        CHARNEXT(self);
        CHARNEXTWHILE(self, SPACE);
        self->isterminated = 1;
        return WS2;
    }
    return NONE;
}

/* comment - \\ or \* ... *\ */
static READTOKENFUNC(readtoken_comment) {
    if(ISCURR(self, "#")) {
        CHARNEXT(self);
        if(ISCURR(self, "*")) {
            CHARNEXT(self);
            while(ISINSIDE(self)) {
                while(CHARNEXT(self) != '*'
                &&    ISINSIDE(self)) incline(self);
                if(CHARNEXT(self) == '#' && ISINSIDE(self)) {
                    CHARNEXT(self);
                    return COMMENT;
                }
            }
            lk_vm_raisecstr(VM,
            "Cannot find *# to close multi-line comment");
        } else {
            CHARNEXTUNTIL(self, NEWLINE);
            self->isterminated = 1;
            return COMMENT;
        }
    }
    return NONE;
}

/* word - identifier, variable */
static READTOKENFUNC(readtoken_word) {
    int lastpos = self->textpos;
    /* if(ISCURR(self, "?")) CHARNEXT(self); */
    if(ISCURR(self, ALPHA)) {
        CHARNEXT(self);
        /*CHARNEXTWHILE(self, LETTER);*/
        while(ISCURR(self, LETTER " ")) {
            if(ISCURR(self, " ")) {
                CHARNEXT(self);
                if(ISNOTCURR(self, LETTER)) {
                    CHARPREV(self);
                    break;
                }
            } else {
                CHARNEXT(self);
            }
        }
        if(ISCURR(self, "=!?")) CHARNEXT(self);
        return WORD;
    }
    self->textpos = lastpos;
    return NONE;
}

/* op - +, - */
static READTOKENFUNC(readtoken_op) {
    if(ISNOTCURR(self, WS ALNUM "{}[]();`'\".")) {
        CHARNEXT(self);
        CHARNEXTUNTIL(self, WS ALNUM "{}[]();`'\".");
        return OP;
    }
    return NONE;
}

/* string - 'hello world' */
static READTOKENFUNC(readtoken_string) {
    uint32_t c;
    if(ISCURR(self, "'")) {
        CHARNEXT(self);
        while(ISINSIDE(self)) {
            while(ISNOTCURR(self, "'\\")) {
                if(ISCURR(self, NEWLINE)) incline(self);
                CHARNEXT(self);
            }
            c = CHARCURR(self);
            CHARNEXT(self);
            switch(c) {
            case '\'': return STRING;
            case '\\': CHARNEXT(self); continue;
            default: lk_vm_raisecstr(VM, "Cannot find ' to close string");
            }
        }
    } else if(ISCURR(self, "\"")) {
        CHARNEXT(self);
        while(ISINSIDE(self)) {
            while(ISNOTCURR(self, "\"\\")) {
                if(ISCURR(self, NEWLINE)) incline(self);
                CHARNEXT(self);
            }
            c = CHARCURR(self);
            CHARNEXT(self);
            switch(c) {
            case '\"': return STRING;
            case '\\': CHARNEXT(self); continue;
            default: lk_vm_raisecstr(VM, "Cannot find \" to close string");
            }
        }
    } else if(ISCURR(self, "`")) {
        CHARNEXT(self);
        while(ISNOTCURR(self, WS "{}[]();`'\".")) CHARNEXT(self);
        return STRING;
    }
    return NONE;
}

/* number - 123_456, 123.456 */
static READTOKENFUNC(readtoken_number) {
    if(ISCURR(self, DIGIT)) {
        int iszero = ISCURR(self, "0");
        CHARNEXT(self);
        if(iszero && ISCURR(self, "c")) {
            CHARNEXT(self);
            if(ISCURR(self, "\\")) CHARNEXT(self);
            CHARNEXT(self);
            return CHAR;
        } else {
            CHARNEXTWHILE(self, "_" DIGIT);
            if(ISCURR(self, ".")) {
                CHARNEXT(self);
                if(!ISCURR(self, DIGIT)) CHARPREV(self);
                else {
                    CHARNEXT(self);
                    CHARNEXTWHILE(self, "_" DIGIT);
                }
            }
            return NUMBER;
        }
    }
    return NONE;
}
static void readtoken(lk_parser_t *self) {
    /* int oldline = self->line; */
    int lastpos = self->textpos;
    tokentype_t type = NONE;
    if((type = readtoken_misc(self))    != NONE
    || (type = readtoken_ws(self))      != NONE
    || (type = readtoken_comment(self)) != NONE
    || (type = readtoken_word(self))    != NONE
    || (type = readtoken_op(self))      != NONE
    || (type = readtoken_string(self))  != NONE
    || (type = readtoken_number(self))  != NONE) {
        lk_string_t *tok = LK_STRING(lk_object_clone(LK_O(self->text)));
        list_slice(LIST(tok), lastpos, self->textpos - lastpos);
        if(type == OP
        &&(list_cmpcstr(LIST(tok), "---") == 0
        || list_cmpcstr(LIST(tok), "|") == 0)) type = FUNC_SEP;
        list_pushptr(self->tokentypes, (void *)type);
        list_pushptr(self->tokenvalues, tok);
        lk_object_addref(LK_O(self), LK_O(tok));
    }
}
static lk_string_t *getnexttoken(lk_parser_t *self, tokentype_t type) {
    list_t *tt = self->tokentypes;
    if(LIST_COUNT(tt) < 1) readtoken(self);
    if((tokentype_t)list_getptr(tt, 0) == type) {
        list_shiftptr(tt);
        return list_shiftptr(self->tokenvalues);
    }
    return NULL;
}

/* whitespace */
static READFUNC(readpad) {
    lk_string_t *tok;
    if((tok = getnexttoken(self, WS2)) != NULL) return 1;
    else if((tok = getnexttoken(self, COMMENT)) != NULL) {
        list_remove(LIST(tok), 0);
        list_pushptr(self->comments, tok);
        return 1;
    }
    return 0;
}

/* expr - object /message[arguments...] +argument ... */
static READFUNC(readobject);
static READFUNC(readmessage);
static READFUNC(readexpr) {
    int old_opcount = self->opcount;
    self->opcount = LIST_COUNT(self->ops);
    if(readobject(self)) {
        lk_instr_t *last;
        while(1) if(!readmessage(self)) break;
        shiftreduce(self, NULL);
        last = list_peekptr(self->words);
        while(last->next != NULL) last = last->next;
        last->opts |= LK_INSTROEND;
        self->opcount = old_opcount;
        return 1;
    } else {
        self->opcount = old_opcount;
        return 0;
    }
}

/* receiver - part 1 */
static lk_instr_t *getexprs(lk_parser_t *self) {
    lk_instr_t *first, *last, *expr;
    first = last = lk_instr_newempty(self);
    while(readexpr(self)) {
        while(last->next != NULL) last = last->next;
        expr = list_popptr(self->words);
        (last->next = expr)->prev = last;
        last = expr;
        if(!(self->isterminated
        || getnexttoken(self, TERMINATOR)) != NONE) break;
    }
    if(first->next != NULL) first->next->prev = NULL;
    return first->next;
}

/* ( expr ) */
static READFUNC(readgroup) {
    if(getnexttoken(self, GROUP_BEGIN) == NULL) return 0;
    if(readexpr(self)) {
        if(getnexttoken(self, GROUP_END)!= NULL) return 1;
        else lk_vm_raisecstr(VM, "Cannot find ) to close group");
    } else {
        lk_vm_raisecstr(VM, "Cannot find an expression following (");
    }
}

/* [ expr1; expr2; expr3; ... ] */
static READFUNC(readlist) {
    if(getnexttoken(self, LIST_BEGIN) == NULL) return 0;
    else {
        lk_instr_t *list = lk_instr_new(self);
        list->type = LK_INSTRTYPE_LIST;
        list->v = LK_O(getexprs(self));
        list_pushptr(self->words, list);
        if(getnexttoken(self, LIST_END) != NULL) return 1;
        else lk_vm_raisecstr(VM, "Cannot find ] to close list");
    }
}

/* { expr1; expr2; expr3; ... } */
static lk_instr_t *applymacros(lk_parser_t *self, lk_instr_t *it);
static READFUNC(readfunc) {
    if(getnexttoken(self, FUNC_BEGIN) == NULL) return 0;
    else {
        lk_instr_t *first, *last, *expr, *sigdef = NULL;
        int isbreak = 0;
        first = last = lk_instr_newempty(self);
        do {
            while(readpad(self)) { }
            if(getnexttoken(self, FUNC_SEP) != NULL) {
                isbreak = 0;
                sigdef = first->next;
                if(sigdef != NULL) sigdef->prev = NULL;
                (last = first)->next = NULL;
            } else if(readexpr(self)) {
                while(last->next != NULL) last = last->next;
                expr = list_popptr(self->words);
                (last->next = expr)->prev = last;
                last = expr;
                isbreak = !(self->isterminated
                || getnexttoken(self, TERMINATOR)) != NONE;
                continue;
            } else {
                break;
            }
            if(isbreak) break;
        } while(1);
        if(first->next != NULL) first->next->prev = NULL;
        first = lk_instr_newfunc(self, first->next);
        LK_KFUNC(first->v)->cf.sigdef = sigdef;
        list_pushptr(self->words, first);
        if(getnexttoken(self, FUNC_END) != NULL) return 1;
        else lk_vm_raisecstr(VM, "Cannot find } to close function");
    }
}

/* msg chain - receiver + receiver */
static READFUNC(readop) {
    lk_string_t *tok = getnexttoken(self, OP);
    if(tok == NULL) return 0;
    else {
        lk_instr_t *op = lk_instr_newmessage(self, tok);
        shiftreduce(self, op);
        if(!readobject(self)) {
            lk_vm_raisecstr(VM, "Cannot find expression after binary operator");
        } else {
            SETOPT(op->opts, LK_INSTROHASMSGARGS);
            return 1;
        }
    }
}
static READFUNC(readmessage) {
    while(readpad(self)) { }
    if(!self->isterminated
    &&(readop(self))) return 1;
    return 0;
}

/* receiver - part 2 */
static READFUNC(readmsg) {
    /* msg[], msg {}, msg[] {}, ... */
    lk_string_t *tok = getnexttoken(self, WORD);
    if(tok == NULL) return 0;
    list_pushptr(self->words, lk_instr_newframemessage(self, tok));
    return 1;
}
static READFUNC(readunaryop) {
    /* + receiver */
    lk_string_t *tok = getnexttoken(self, OP);
    if(tok == NULL) return 0;
    if(!readobject(self)) {
        lk_vm_raisecstr(VM, "Cannot find expression after unary operator");
    } else {
        lk_instr_t *arg = list_peekptr(self->words);
        /* /arg[] -> /arg[] */
        if(list_cmpcstr(LIST(tok), "/") == 0) {
            arg->type = LK_INSTRTYPE_SELFMSG;
        /* +arg[] -> arg[] /+[] */
        } else {
            lk_instr_t *op = lk_instr_newmessage(self, getunaryop(self, tok));
            while(arg->next != NULL) arg = arg->next;
            (arg->next = op)->prev = arg;
        }
        return 1;
    }
}
static READFUNC(readchar) {
    lk_string_t *tok = getnexttoken(self, CHAR);
    if(tok == NULL) return 0;
    else {
        lk_string_unescape(tok);
        list_pushptr(self->words,
        lk_instr_newchar(self, list_getuchar(LIST(tok), 2)));
        return 1;
    }
}
static READFUNC(readnumber) {
    lk_string_t *tok = getnexttoken(self, NUMBER);
    if(tok == NULL) return 0;
    else {
        numberifn_t num;
        switch(number_new(0, LIST(tok), &num)) {
        case NUMBERTYPE_INT:
            list_pushptr(self->words, lk_instr_newfi(self, num.i));
            break;
        case NUMBERTYPE_FLOAT:
            list_pushptr(self->words, lk_instr_newff(self, num.f));
            break;
        default:
            BUG("Invalid number type while trying to parse code.\n");
        }
        return 1;
    }
}
static READFUNC(readstring) {
    lk_string_t *tok = getnexttoken(self, STRING);
    if(tok == NULL) return 0;
    else {
        list_slice(LIST(tok), 1, -1);
        lk_string_unescape(tok);
        list_pushptr(self->words, lk_instr_newstring(self, tok));
        return 1;
    }
}
static READFUNC(readobject) {
    while(readpad(self)) { }
    self->isterminated = 0;
    if(readchar(self)
    || readnumber(self)
    || readstring(self)
    || readgroup(self)
    || readlist(self)
    || readfunc(self)
    || readmsg(self)
    || readunaryop(self)
    ) {
        while(1) {
            while(readpad(self)) { }
            if(!self->isterminated) {
                lk_instr_t *args;
                if(!readlist(self)) args = NULL;
                else args = list_popptr(self->words);
                while(readpad(self)) { }
                if(!self->isterminated && readfunc(self)) {
                    lk_instr_t *f = list_popptr(self->words);
                    if(args == NULL) args = lk_instr_newarglist(self, f);
                    else {
                        lk_instr_t *a = LK_INSTR(args->v);
                        while(a->next != NULL) a = a->next;
                        (a->next = f)->prev = a;
                    }
                }
                if(args != NULL) {
                    lk_instr_t *obj = list_peekptr(self->words);
                    args->type = LK_INSTRTYPE_APPLY;
                    while(obj->next != NULL) obj = obj->next;
                    (obj->next = args)->prev = obj;
                    if(obj->type == LK_INSTRTYPE_FRAMEMSG
                    ) obj->opts |= LK_INSTROHASMSGARGS;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        return 1;
    }
    return 0;
}

/* real parse */
lk_instr_t *lk_parser_parse(lk_parser_t *self, const lk_string_t *text) {
    self->text = text;
    self->error = NULL;
    self->textpos = 0;
    self->line = 1;
    self->column = 1;
    list_clear(self->tokentypes);
    list_clear(self->tokenvalues);
    list_clear(self->words);
    list_clear(self->ops);
    list_clear(self->comments);
    self->opcount = 0;
    self->isterminated = 0;
    if(ISCURR(self, "#")) {
        CHARNEXT(self);
        CHARNEXTUNTIL(self, NEWLINE);
    }
    return lk_instr_newmore(self);
}
lk_instr_t *lk_parser_getmore(lk_parser_t *self) {
    lk_instr_t *first = NULL;
    if(readexpr(self)) {
        lk_instr_t *instr = first = list_popptr(self->words);
        lk_string_t *tok;
        while(instr->next != NULL) instr = instr->next;
        (instr->next = lk_instr_newmore(self))->prev = instr;
        first = applymacros(self, first);
        /* printf("<<< "); lk_instr_print(first); printf("\n"); */
        if(self->isterminated) goto done;
        if((tok = getnexttoken(self, TERMINATOR)) != NULL) {
            goto done;
        }
        lk_vm_raisecstr(VM, "Cannot find terminator after expression");
    }
    done:
    assert(LIST_COUNT(self->words) == 0);
    assert(LIST_COUNT(self->ops) == 0);
    return first;
}
static lk_instr_t *applymacros(lk_parser_t *self, lk_instr_t *it) {
    lk_instr_t *first = it;
    for(; it != NULL; it = it->next) {
        if(it->type == LK_INSTRTYPE_FUNC) {
            LK_KFUNC(it->v)->cf.sigdef= applymacros(
            self, LK_KFUNC(it->v)->cf.sigdef);
            LK_KFUNC(it->v)->first = applymacros(self, LK_KFUNC(it->v)->first);
        } else if(it->type == LK_INSTRTYPE_APPLY
               || it->type == LK_INSTRTYPE_LIST) {
            it->v = LK_O(applymacros(self, LK_INSTR(it->v)));
            /*
        } else if(it->type == LK_INSTRTYPE_FRAMEMSG
               && list_cmpcstr(LIST(it->v), ".") == 0) {
            lk_instr_t *msg = it->next;
            if(msg != NULL && msg->type == LK_INSTRTYPE_APPLYMSG) {
                msg->type = LK_INSTRTYPE_SELFMSG;
                if((msg->prev = it->prev) != NULL) msg->prev->next = msg;
                else first = msg;
            }
            */
        } else if(it->type == LK_INSTRTYPE_APPLYMSG
               && (list_cmpcstr(LIST(it->v), ".define!") == 0
               || list_cmpcstr(LIST(it->v), ".assign!") == 0
               || list_cmpcstr(LIST(it->v), ".define_assign!") == 0)) {
            lk_instr_t *name = it->prev, *args = it->next;
            if(name->type == LK_INSTRTYPE_SELFMSG
            || name->type == LK_INSTRTYPE_FRAMEMSG
            || name->type == LK_INSTRTYPE_APPLYMSG) {
                /* var[] /:= @[1 ] -> := @['var' 1 ] */
                lk_instr_t *nextop = args->next;
                it->type = name->type;
                if((it->prev = name->prev) != NULL) it->prev->next = it;
                else first = it;
                name->type = LK_INSTRTYPE_STRING;
                (name->next = LK_INSTR(args->v))->prev = name;
                args->v = LK_O(name);
                /* var[] /: @[Object ] /= @[1 ] -> := @['var' Object 1 ] */
                if(nextop != NULL
                && nextop->type == LK_INSTRTYPE_APPLYMSG
                && list_cmpcstr(LIST(nextop->v), ".assign!") == 0) {
                    lk_instr_t *a, *nextargs = nextop->next;
                    it->v = LK_O(lk_string_newfromcstr(
                    LK_VM(it), ".define_assign!"));
                    (it->next = nextargs)->prev = it;
                    a = LK_INSTR(args->v);
                    while(a->next != NULL) a = a->next;
                    (a->next = LK_INSTR(nextargs->v))->prev = a;
                    nextargs->v = args->v;
                }
            } else if(name->type == LK_INSTRTYPE_APPLY) {
                /* list @[1 ] /= @[2 ] -> list[] /@= @[1 2 ] */
                lk_instr_t *a, *atargs = name;
                name = name->prev;
                name->opts &= ~LK_INSTROHASMSGARGS;
                (name->next = it)->prev = name;
                it->v = LK_O(lk_string_newfromcstr(LK_VM(it), "set!"));
                a = LK_INSTR(atargs->v);
                while(a->next != NULL) a = a->next;
                (a->next = LK_INSTR(args->v))->prev = a;
                args->v = atargs->v;
            }
        } else if(it->type == LK_INSTRTYPE_APPLYMSG
               && list_cmpcstr(LIST(it->v), "else") == 0) {
            /* 1 /?? @[2 ]  /!! @[3 ]   -> 1 /?? @[2 3 ] */
            /*   op  add(a) it  args(b) ->   op  args    */
            lk_instr_t *args = it->next;
            if(args->type == LK_INSTRTYPE_APPLY) {
                lk_instr_t *add = it->prev;
                if(add->type == LK_INSTRTYPE_APPLY) {
                    lk_instr_t *op = add->prev;
                    lk_instr_t *a = LK_INSTR(add->v), *b = LK_INSTR(args->v);
                    (op->next = args)->prev = op;
                    while(a->next != NULL) a = a->next;
                    /* convert b to func if op is ?? for short-circuiting */
                    if(b->type != LK_INSTRTYPE_FUNC
                    && list_cmpcstr(LIST(op->v), "then") == 0) {
                        b = lk_instr_newfunc(self, b);
                    }
                    (a->next = b)->prev = a;
                    args->v = LK_O(a);
                }
            }
        }
    }
    return first;
}