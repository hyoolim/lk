#include "lib.h"

// ext map - types
static LK_OBJ_DEFMARKFUNC(mark_instr) {
    mark(LK_OBJ(LK_INSTR(self)->prev));
    mark(LK_OBJ(LK_INSTR(self)->next));
    mark(LK_OBJ(LK_INSTR(self)->rsrc));
    mark(LK_INSTR(self)->v);

    if (LK_INSTR(self)->comment != NULL)
        mark(LK_OBJ(LK_INSTR(self)->comment));
}

void lk_instr_type_init(lk_vm_t *vm) {
    vm->t_instr = lk_obj_alloc_with_size(vm->t_obj, sizeof(lk_instr_t));
    lk_obj_set_mark_func(vm->t_instr, mark_instr);
}

// info
lk_num_t *lk_instr_column(lk_obj_t *self) {
    return lk_num_new(VM, LK_INSTR(self)->column);
}

lk_num_t *lk_instr_line(lk_obj_t *self) {
    return lk_num_new(VM, LK_INSTR(self)->line);
}

lk_str_t *lk_instr_message(lk_obj_t *self) {
    lk_instr_t *instr = LK_INSTR(self);

    do {
        switch (instr->type) {
        case LK_INSTRTYPE_APPLYMSG:
        case LK_INSTRTYPE_SCOPEMSG:
        case LK_INSTRTYPE_SELFMSG:
            return LK_STRING(instr->v);
        default:
            instr = instr->prev;
        }
    } while (instr != NULL);
    return LK_STRING(NIL);
}

lk_str_t *lk_instr_resource(lk_obj_t *self) {
    return LK_INSTR(self)->rsrc;
}

// bind all c funcs to lk equiv
void lk_instr_lib_init(lk_vm_t *vm) {
    lk_obj_t *instr = vm->t_instr;
    lk_object_set(vm->t_vm, "Instruction", instr);
    lk_obj_set_cfield(instr, "_next", instr, offsetof(lk_instr_t, next));
    lk_obj_set_cfield(instr, "_previous", instr, offsetof(lk_instr_t, prev));

    // info
    lk_obj_set_cfunc_creturn(instr, "COLUMN", lk_instr_column, NULL);
    lk_obj_set_cfunc_creturn(instr, "LINE", lk_instr_line, NULL);
    lk_obj_set_cfunc_creturn(instr, "MESSAGE", lk_instr_message, NULL);
    lk_obj_set_cfunc_creturn(instr, "RESOURCE", lk_instr_resource, NULL);
}

// new
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
    return instr_new(parser, (enum lk_instrtype_t)0);
}

lk_instr_t *lk_instr_new_more(lk_parser_t *parser) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_MORE);
    new->v = LK_OBJ(parser);
    return new;
}

lk_instr_t *lk_instr_new_func(lk_parser_t *parser, lk_instr_t *first) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_FUNC);
    new->v = LK_OBJ(lk_lfunc_new(LK_VM(parser), NULL, first));
    return new;
}

lk_instr_t *lk_instr_new_arglist(lk_parser_t *parser, lk_instr_t *func) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_APPLY);
    new->v = LK_OBJ(func);
    return new;
}

lk_instr_t *lk_instr_new_str(lk_parser_t *parser, lk_str_t *s) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_STRING);
    new->v = LK_OBJ(s); // lk_str_new_from_darray(LK_VM(parser), s));
    return new;
}

lk_instr_t *lk_instr_new_number(lk_parser_t *parser, double num) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_NUMBER);
    new->v = LK_OBJ(lk_num_new(LK_VM(parser), num));
    return new;
}

lk_instr_t *lk_instr_new_char(lk_parser_t *parser, uint32_t c) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_CHAR);
    new->v = LK_OBJ(lk_char_new(LK_VM(parser), c));
    return new;
}

lk_instr_t *lk_instr_new_empty(lk_parser_t *parser) {
    return instr_new(parser, (enum lk_instrtype_t)0);
}

lk_instr_t *lk_instr_new_message(lk_parser_t *parser, lk_str_t *name) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_APPLYMSG);
    new->v = LK_OBJ(name);
    {
        vec_t *cs = parser->comments;
        lk_str_t *c = vec_ptr_remove(cs, 0);

        while (cs->length > 0) {
            vec_concat(VEC(c), VEC(vec_ptr_remove(cs, 0)));
        }
        new->comment = c;
    }
    return new;
}

lk_instr_t *lk_instr_new_scope_message(lk_parser_t *parser, lk_str_t *name) {
    lk_instr_t *new = instr_new(parser, LK_INSTRTYPE_SCOPEMSG);
    new->v = LK_OBJ(name);
    return new;
}

// info
void lk_instr_print(lk_instr_t *self) {
    if (self == NULL)
        return;

    switch (self->type) {
    case LK_INSTRTYPE_FUNC:
        printf("{");
        lk_instr_print(LK_LFUNC(self->v)->first);
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
        vec_print_tostream(VEC(self->v), stdout);
        printf("'");
        break;
    case LK_INSTRTYPE_NUMBER:
        printf("%f", CNUMBER(self->v));
        break;
    case LK_INSTRTYPE_CHAR:
        printf("%c", CHAR(self->v));
        break;
    case LK_INSTRTYPE_APPLYMSG:
        printf(".");
        vec_print_tostream(VEC(self->v), stdout);

        if (!(self->opts & LK_INSTROHASMSGARGS))
            printf("[]");
        break;
    case LK_INSTRTYPE_SCOPEMSG:
        vec_print_tostream(VEC(self->v), stdout);

        if (!(self->opts & LK_INSTROHASMSGARGS))
            printf("[]");
        break;
    case LK_INSTRTYPE_SELFMSG:
        printf(".");
        vec_print_tostream(VEC(self->v), stdout);

        if (!(self->opts & LK_INSTROHASMSGARGS))
            printf("[]");
        break;
    case LK_INSTRTYPE_MORE:
        break;
    default:
        printf("?%i", (int)self->type);
        break;
    }

    if (self->comment != NULL) {
        printf(" #*");
        vec_print_tostream(VEC(self->comment), stdout);
        printf(" *#");
    }
    printf(self->opts & LK_INSTROEND ? "; " : " ");
    lk_instr_print(self->next);
}
