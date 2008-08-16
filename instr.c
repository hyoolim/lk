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
LK_EXT_DEFINIT(lk_Instr_extinittypes) {
    vm->t_instr = lk_Object_allocwithsize(vm->t_obj, sizeof(lk_Instr_t));
    lk_Object_setmarkfunc(vm->t_instr, mark__instr);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(column) {
    RETURN(lk_Fi_new(VM, INSTR->column)); }
LK_LIBRARY_DEFINECFUNCTION(line) {
    RETURN(lk_Fi_new(VM, INSTR->line)); }
LK_LIBRARY_DEFINECFUNCTION(message) {
    lk_Instr_t *instr = INSTR;
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
LK_LIBRARY_DEFINECFUNCTION(resource) {
    RETURN(INSTR->rsrc); }
LK_EXT_DEFINIT(lk_Instr_extinitfuncs) {
    lk_Object_t *instr = vm->t_instr;
    lk_Library_set(vm->t_vm, "Instruction", instr);
    lk_Library_cfield(instr, ".next", instr, offsetof(lk_Instr_t, next));
    lk_Library_cfield(instr, ".previous", instr, offsetof(lk_Instr_t, prev));
    lk_Library_setCFunction(instr, "COLUMN", column, NULL);
    lk_Library_setCFunction(instr, "LINE", line, NULL);
    lk_Library_setCFunction(instr, "MESSAGE", message, NULL);
    lk_Library_setCFunction(instr, "RESOURCE", resource, NULL);
}

/* new */
static lk_Instr_t *instr_new(lk_Parser_t *parser, enum lk_Instrtype_t type) {
    lk_Vm_t *vm = LK_VM(parser);
    lk_Instr_t *self = LK_INSTR(lk_Object_alloc(vm->t_instr));
    self->type = type;
    self->next = NULL;
    self->rsrc = vm->rsrc->rsrc;
    self->line = parser->line;
    self->column = parser->column;
    return self;
}
lk_Instr_t *lk_Instr_new(lk_Parser_t *parser) {
    return instr_new(parser, (enum lk_Instrtype_t)NULL);
}
lk_Instr_t *lk_Instr_newmore(lk_Parser_t *parser) {
    lk_Instr_t *new = instr_new(parser, LK_INSTRTYPE_MORE);
    new->v = LK_OBJ(parser);
    return new;
}
lk_Instr_t *lk_Instr_newfunc(lk_Parser_t *parser, lk_Instr_t *first) {
    lk_Instr_t *new = instr_new(parser, LK_INSTRTYPE_FUNC);
    new->v = LK_OBJ(lk_Kfunc_new(LK_VM(parser), NULL, first));
    return new;
}
lk_Instr_t *lk_Instr_newarglist(lk_Parser_t *parser, lk_Instr_t *func) {
    lk_Instr_t *new = instr_new(parser, LK_INSTRTYPE_APPLY);
    new->v = LK_OBJ(func);
    return new;
}
lk_Instr_t *lk_Instr_newstring(lk_Parser_t *parser, lk_String_t *s) {
    lk_Instr_t *new = instr_new(parser, LK_INSTRTYPE_STRING);
    new->v = LK_OBJ(s); /* lk_String_newfromlist(LK_VM(parser), s)); */
    return new;
}
lk_Instr_t *lk_Instr_newfi(lk_Parser_t *parser, int i) {
    lk_Instr_t *new = instr_new(parser, LK_INSTRTYPE_FIXINT);
    new->v = LK_OBJ(lk_Fi_new(LK_VM(parser), i));
    return new;
}
lk_Instr_t *lk_Instr_newchar(lk_Parser_t *parser, uint32_t c) {
    lk_Instr_t *new = instr_new(parser, LK_INSTRTYPE_CHAR);
    new->v = LK_OBJ(lk_Char_new(LK_VM(parser), c));
    return new;
}
lk_Instr_t *lk_Instr_newempty(lk_Parser_t *parser) {
    return instr_new(parser, (enum lk_Instrtype_t)NULL);
}
lk_Instr_t *lk_Instr_newff(lk_Parser_t *parser, double f) {
    lk_Instr_t *new = instr_new(parser, LK_INSTRTYPE_FIXF);
    new->v = LK_OBJ(lk_Fr_new(LK_VM(parser), f));
    return new;
}
lk_Instr_t *lk_Instr_newmessage(lk_Parser_t *parser, lk_String_t *name) {
    lk_Instr_t *new = instr_new(parser, LK_INSTRTYPE_APPLYMSG);
    new->v = LK_OBJ(name);
    {
        Sequence_t *cs = parser->comments;
        lk_String_t *c = Sequence_removeptr(cs, 0);
        while(cs->count > 0) {
            Sequence_concat(LIST(c), LIST(Sequence_removeptr(cs, 0)));
        }
        new->comment = c;
    }
    return new;
}
lk_Instr_t *lk_Instr_newframemessage(lk_Parser_t *parser, lk_String_t *name) {
    lk_Instr_t *new = instr_new(parser, LK_INSTRTYPE_FRAMEMSG);
    new->v = LK_OBJ(name);
    return new;
}

/* info */
void lk_Instr_print(lk_Instr_t *self) {
    if(self == NULL) return;
    switch(self->type) {
    case LK_INSTRTYPE_FUNC:
        printf("{");
        lk_Instr_print(LK_KFUNC(self->v)->first);
        printf("}");
        break;
    case LK_INSTRTYPE_APPLY:
        printf("@[");
        lk_Instr_print(LK_INSTR(self->v));
        printf("]");
        break;
    case LK_INSTRTYPE_LIST:
        printf("[");
        lk_Instr_print(LK_INSTR(self->v));
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
    lk_Instr_print(self->next);
}
