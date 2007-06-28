#include "instr.h"
#include "char.h"
#include "ext.h"
#include "fixnum.h"
#include "string.h"
#define INSTR (LK_INSTR(self))

/* ext map - types */
static LK_OBJ_DEFMARKFUNC(mark__instr) {
    mark(LK_OBJ(INSTR->prev));
    mark(LK_OBJ(INSTR->next));
    mark(LK_OBJ(INSTR->rsrc));
    mark(INSTR->v);
    if(INSTR->comment != NULL) mark(LK_OBJ(INSTR->comment));
}
LK_EXT_DEFINIT(lk_instr_extinittypes) {
    vm->t_instr = lk_obj_allocwithsize(vm->t_obj, sizeof(lk_instr_t));
    lk_obj_setmarkfunc(vm->t_instr, mark__instr);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(column) {
    RETURN(lk_fi_new(VM, INSTR->column)); }
static LK_EXT_DEFCFUNC(line) {
    RETURN(lk_fi_new(VM, INSTR->line)); }
static LK_EXT_DEFCFUNC(message) {
    lk_instr_t *instr = INSTR;
    do {
        switch(instr->type) {
        case LK_INSTRTYPE_APPLYMSG:
        case LK_INSTRTYPE_FRAMEMSG:
        case LK_INSTRTYPE_SELFMSG: RETURN(instr->v);
        default: instr = instr->prev;
        }
    } while(instr != NULL);
    RETURN(N);
}
static LK_EXT_DEFCFUNC(resource) {
    RETURN(INSTR->rsrc); }
LK_EXT_DEFINIT(lk_instr_extinitfuncs) {
    lk_obj_t *instr = vm->t_instr;
    lk_ext_set(vm->t_vm, "Instruction", instr);
    lk_ext_cfield(instr, ".next", instr, offsetof(lk_instr_t, next));
    lk_ext_cfield(instr, ".previous", instr, offsetof(lk_instr_t, prev));
    lk_ext_cfunc(instr, "COLUMN", column, NULL);
    lk_ext_cfunc(instr, "LINE", line, NULL);
    lk_ext_cfunc(instr, "MESSAGE", message, NULL);
    lk_ext_cfunc(instr, "RESOURCE", resource, NULL);
}

/* new */
static lk_instr_t *instr_new(lk_parser_t *parser, enum lk_instrtype_t type) {
    lk_vm_t *vm = LK_VM(parser);
    lk_instr_t *self = LK_INSTR(lk_obj_alloc(vm->t_instr));
    self->type = type;
    self->next = NULL;
    self->rsrc = vm->rsrc->rsrc;
    self->line = parser->line;
    self->column = parser->column;
    return self;
}
lk_instr_t *lk_instr_new(lk_parser_t *parser) {
    return instr_new(parser, (enum lk_instrtype_t)NULL);
}
lk_instr_t *lk_instr_newmore(lk_parser_t *parser) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_MORE);
    new->v = LK_OBJ(parser);
    return new;
}
lk_instr_t *lk_instr_newfunc(lk_parser_t *parser, lk_instr_t *first) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_FUNC);
    new->v = LK_OBJ(lk_kfunc_new(LK_VM(parser), NULL, first));
    return new;
}
lk_instr_t *lk_instr_newarglist(lk_parser_t *parser, lk_instr_t *func) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_APPLY);
    new->v = LK_OBJ(func);
    return new;
}
/*
static lk_string_t *instr_newsymbol(lk_vm_t *vm, string_t *s) {
    lk_string_t *new = lk_string_newfromlist(vm, s);
    setitem_t *i = set_get(vm->symbols, new);
    if(i == NULL) set_set(vm->symbols, new);
    else {
        lk_obj_free(LK_OBJ(new));
        new = LK_STRING(i->key);
    }
    return new;
}
 */
lk_instr_t *lk_instr_newstring(lk_parser_t *parser, lk_string_t *s) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_STRING);
    new->v = LK_OBJ(s); /* lk_string_newfromlist(LK_VM(parser), s)); */
    return new;
}
lk_instr_t *lk_instr_newfi(lk_parser_t *parser, int i) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_FIXINT);
    new->v = LK_OBJ(lk_fi_new(LK_VM(parser), i));
    return new;
}
lk_instr_t *lk_instr_newchar(lk_parser_t *parser, uint32_t c) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_CHAR);
    new->v = LK_OBJ(lk_char_new(LK_VM(parser), c));
    return new;
}
lk_instr_t *lk_instr_newempty(lk_parser_t *parser) {
    return instr_new(parser, (enum lk_instrtype_t)NULL);
}
lk_instr_t *lk_instr_newff(lk_parser_t *parser, double f) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_FIXF);
    new->v = LK_OBJ(lk_fr_new(LK_VM(parser), f));
    return new;
}
lk_instr_t *lk_instr_newmessage(lk_parser_t *parser, lk_string_t *name) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_APPLYMSG);
    new->v = LK_OBJ(name); /* instr_newsymbol(LK_VM(parser), name)); */
    {
        list_t *cs = parser->comments;
        lk_string_t *c = list_removeptr(cs, 0);
        while(cs->count > 0) {
            list_concat(LIST(c), LIST(list_removeptr(cs, 0)));
        }
        new->comment = c;
    }
    return new;
}
lk_instr_t *lk_instr_newframemessage(lk_parser_t *parser, lk_string_t *name) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_FRAMEMSG);
    new->v = LK_OBJ(name); /* instr_newsymbol(LK_VM(parser), name)); */
    return new;
}

/* info */
void lk_instr_print(lk_instr_t *self) {
    if(self == NULL) return;
    switch(self->type) {
    case LK_INSTRTYPE_FUNC:
        printf("{");
        lk_instr_print(LK_KFUNC(self->v)->first);
        printf("}");
        break;
    case LK_INSTRTYPE_APPLY:
        printf("@[");
        lk_instr_print(LK_INSTR(self->v));
        printf("]");
        break;
    case LK_INSTRTYPE_LIST:
        printf("[");
        lk_instr_print(LK_INSTR(self->v));
        printf("]");
        break;
    case LK_INSTRTYPE_STRING:
        printf("'");
        string_print(LIST(self->v), stdout);
        printf("'");
        break;
    case LK_INSTRTYPE_FIXINT:
        printf("%i", INT(self->v));
        break;
    case LK_INSTRTYPE_FIXF:
        printf("%f", DBL(self->v));
        break;
    case LK_INSTRTYPE_CHAR:
        printf("%c", CHAR(self->v));
        break;
    case LK_INSTRTYPE_APPLYMSG:
        printf("/");
        string_print(LIST(self->v), stdout);
        if(!(self->opts & LK_INSTROHASMSGARGS)) printf("[]");
        break;
    case LK_INSTRTYPE_FRAMEMSG:
        string_print(LIST(self->v), stdout);
        if(!(self->opts & LK_INSTROHASMSGARGS)) printf("[]");
        break;
    case LK_INSTRTYPE_SELFMSG:
        printf("./");
        string_print(LIST(self->v), stdout);
        if(!(self->opts & LK_INSTROHASMSGARGS)) printf("[]");
        break;
    case LK_INSTRTYPE_MORE:
        break;
    default:
        printf("?%i", (int)self->type);
        break;
    }
    if(self->comment != NULL) {
        printf(" #*");
        string_print(LIST(self->comment), stdout);
        printf(" *#");
    }
    printf(self->opts & LK_INSTROEND ? "; " : " ");
    lk_instr_print(self->next);
}
