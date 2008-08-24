#include "vm.h"
#include "bool.h"
#include "char.h"
#include "charset.h"
#include "env.h"
#include "error.h"
#include "ext.h"
#include "file.h"
#include "vector.h"
#include "number.h"
#include "folder.h"
#include "scope.h"
#include "func.h"
#include "gc.h"
#include "seq.h"
#include "instr.h"
#include "list.h"
#include "map.h"
#include "object.h"
#include "parser.h"
#include "random.h"
#include "socket.h"
#include "string.h"
#include <errno.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* ext map - types */
void lk_vm_typeinit(lk_vm_t *vm) {
    vm->t_vm = lk_object_alloc(vm->t_object);
}

/* ext map - funcs */
static void exit_vm(lk_object_t *self, lk_scope_t *local) {
    lk_vm_exit(VM);
    DONE;
}
static void fork_vm(lk_object_t *self, lk_scope_t *local) {
    pid_t child = fork();
    if(child == -1) lk_vm_raiseerrno(VM);
    RETURN(lk_number_new(VM, (int)child));
}
static void fork_vm_f(lk_object_t *self, lk_scope_t *local) {
    pid_t child = fork();
    if(child == -1) lk_vm_raiseerrno(VM);
    if(child > 0) RETURN(lk_number_new(VM, (int)child));
    else {
        lk_kfunc_t *kf = LK_KFUNC(ARG(0));
        lk_scope_t *fr = lk_scope_new(VM);
        fr->first = fr->next = kf->first;
        fr->receiver = fr->self = self;
        fr->func = LK_OBJ(kf);
        fr->returnto = NULL;
        fr->o.parent = LK_OBJ(kf->scope);
        lk_vm_doevalfunc(VM);
        lk_vm_exit(VM);
        DONE;
    }
}
static void sleep_vm_number(lk_object_t *self, lk_scope_t *local) {
    usleep((unsigned long)(CNUMBER(ARG(0)) * 1000000));
    RETURN(self);
}
static void seconds_since_epoch_vm(lk_object_t *self, lk_scope_t *local) {
    struct timeval now;
    gettimeofday(&now, NULL);
    RETURN(lk_number_new(VM, now.tv_sec + now.tv_usec / 1000000.0));
}
static void seconds_west_of_utc_vm(lk_object_t *self, lk_scope_t *local) {
    time_t raw;
    struct tm gm, l;
    long offset;
    raw = time(NULL);
    gm = *gmtime(&raw);
    l = *localtime(&raw);
    offset = l.tm_sec - gm.tm_sec;
    offset += (l.tm_min - gm.tm_min) * 60;
    offset += (l.tm_hour - gm.tm_hour) * 3600;
    if(l.tm_year < gm.tm_year || l.tm_mon < gm.tm_mon || l.tm_mday < gm.tm_mday) {
        offset -= 86400;
    }
    RETURN(lk_number_new(VM, offset));
}
static void system_vm(lk_object_t *self, lk_scope_t *local) {
    pid_t child = fork();
    if(child == -1) lk_vm_raiseerrno(VM);
    if(child > 0) {
        int status;
        waitpid(child, &status, 0);
        RETURN(lk_number_new(VM, WEXITSTATUS(status)));
    } else {
        int i, c = local->argc;
        char **args = memory_alloc(sizeof(char *) * (c + 1));
        for(i = 0; i < c; i ++) {
            args[i] = (char *)darray_toCString(DARRAY(ARG(i)));
        }
        execvp(args[0], args);
        lk_vm_raiseerrno(VM);
    }
}
static void system2_vm_str(lk_object_t *self, lk_scope_t *local) {
    FILE *out = popen(darray_toCString(DARRAY(ARG(0))), "r");
    if(out != NULL) {
        char ret[4096];
        char *line = fgets(ret, 4096, out);
        if(line != NULL) RETURN(lk_string_newFromCString(VM, line));
    }
    RETURN(NIL);
}
static void wait_vm(lk_object_t *self, lk_scope_t *local) {
    int status;
    pid_t child = wait(&status);
    RETURN(lk_number_new(VM, (int)child));
}
void lk_vm_libinit(lk_vm_t *vm) {
    lk_object_t *tvm = vm->t_vm, *f = vm->t_func, *number = vm->t_number, *str = vm->t_string;
    lk_lib_setGlobal("VirtualMachine", tvm);
    lk_lib_setCFunc(tvm, "exit", exit_vm, NULL);
    lk_lib_setCFunc(tvm, "fork", fork_vm, NULL);
    lk_lib_setCFunc(tvm, "fork", fork_vm_f, f, NULL);
    lk_lib_setCFunc(tvm, "sleep", sleep_vm_number, number, NULL);
    lk_lib_setCFunc(tvm, "seconds since epoch", seconds_since_epoch_vm, NULL);
    lk_lib_setCFunc(tvm, "seconds west of utc", seconds_west_of_utc_vm, NULL);
    lk_lib_setCFunc(tvm, "system", system_vm, -1);
    lk_lib_setCFunc(tvm, "system2", system2_vm_str, str, NULL);
    lk_lib_setCFunc(tvm, "wait", wait_vm, NULL);
}

/* new */
lk_vm_t *lk_vm_new(void) {
    lk_vm_t *self = memory_alloc(sizeof(lk_vm_t));

    /* must be loaded before other primitive types */
    lk_object_typeinit(self);
    lk_gc_typeinit(self);
    lk_vm_typeinit(self);
    lk_seq_typeinit(self);
    lk_map_typeinit(self);
    lk_string_typeinit(self);

    /* init all other primitive types */
    lk_bool_typeinit(self);
    lk_char_typeinit(self);
    lk_charset_typeinit(self);
    lk_error_typeinit(self);
    lk_file_typeinit(self);
    lk_folder_typeinit(self);
    lk_vector_typeinit(self);
    lk_number_typeinit(self);
    lk_scope_typeinit(self);
    lk_func_typeinit(self);
    lk_instr_typeinit(self);
    lk_list_typeinit(self);
    lk_parser_typeinit(self);

    /* init rest of the fields in vm */
    self->global = LK_SCOPE(lk_object_alloc(self->t_scope));
    self->currentScope = self->global;
    lk_lib_setGlobal(".string.class", LK_OBJ(self->str_type = lk_string_newFromCString(self, ".CLASS")));
    lk_lib_setGlobal(".string.forward", LK_OBJ(self->str_forward = lk_string_newFromCString(self, ".forward")));
    lk_lib_setGlobal(".string.rescue", LK_OBJ(self->str_rescue = lk_string_newFromCString(self, ".rescue")));
    lk_lib_setGlobal(".string.on assign", LK_OBJ(self->str_onassign = lk_string_newFromCString(self, ".on_assign")));
    lk_lib_setGlobal(".string.at", LK_OBJ(self->str_at = lk_string_newFromCString(self, "at")));
    lk_lib_setGlobal(".string.slash", LK_OBJ(self->str_filesep = lk_string_newFromCString(self, "/")));

    /* attach all funcs to primitive types */
    lk_bool_libinit(self);
    lk_char_libinit(self);
    lk_charset_libinit(self);
    lk_map_libinit(self);
    lk_error_libinit(self);
    lk_file_libinit(self);
    lk_folder_libinit(self);
    lk_vector_libinit(self);
    lk_number_libinit(self);
    lk_scope_libinit(self);
    lk_func_libinit(self);
    lk_gc_libinit(self);
    lk_instr_libinit(self);
    lk_list_libinit(self);
    lk_object_libinit(self);
    lk_parser_libinit(self);
    lk_string_libinit(self);
    lk_vm_libinit(self);
    lk_seq_libinit(self);

    /* extra libs */
    lk_env_extinit(self);
    lk_library_extinit(self);
    lk_random_extinit(self);
    lk_socket_extinit(self);
    return self;
}
void lk_vm_free(lk_vm_t *self) {
    lk_gc_t *gc = self->gc;
    lk_objGroup_remove(LK_OBJ(gc));
    lk_objGroup_freeAll(gc->unused);
    lk_objGroup_freeAll(gc->pending);
    lk_objGroup_freeAll(gc->used);
    lk_object_justfree(LK_OBJ(gc));
    memory_free(self);

    /*
    fprintf(stderr, "allocsize: %i\n", memory_allocsize());
    fprintf(stderr, "alloctotal: %i\n", memory_alloctotal());
    fprintf(stderr, "allocpeak: %i\n", memory_allocpeak());
    fprintf(stderr, "allocused: %i\n", memory_allocused());
    */
}

/* eval */
static lk_scope_t *eval(lk_vm_t *self, lk_string_t *code) {
    lk_parser_t *p = lk_parser_new(self);
    lk_instr_t *func = lk_parser_parse(p, code);
    lk_scope_t *fr = lk_scope_new(self);
    fr->first = fr->next = func;
    fr->returnto = NULL;
    lk_vm_doevalfunc(self);
    return fr;
}
lk_scope_t *lk_vm_evalfile(lk_vm_t *self, const char *file, const char *base) {
    lk_string_t *filename = lk_string_newFromCString(self, file);
    if(base != NULL) {
        lk_string_t *root = lk_string_newFromCString(self, base);
        int pos, i, pslen = LIST_COUNT(DARRAY(self->str_filesep));
        pos = darray_findDArray(DARRAY(root), DARRAY(self->str_filesep), 0);
        if(pos > 0) {
            lk_string_t *orig = filename;
            root = lk_string_newFromDArray(self, DARRAY(root));
            pos += pslen;
            while((i = darray_findDArray(
            DARRAY(root), DARRAY(self->str_filesep), pos)) > 0) pos = i + pslen;
            darray_slice(DARRAY(root), 0, pos);
            darray_resizeitem(DARRAY(root), DARRAY(orig));
            darray_concat(DARRAY(root), DARRAY(orig));
            filename = root;
        }
    }
    {
        lk_scope_t *fr;
        struct lk_rsrcchain rsrc;
        FILE *stream;
        const char *cfilename = darray_toCString(DARRAY(filename));
        rsrc.isstring = 0;
        rsrc.rsrc = filename;
        rsrc.prev = self->rsrc;
        self->rsrc = &rsrc;
        stream = fopen(cfilename, "r");
        if(stream != NULL) {
            darray_t *src = string_allocfromfile(stream);
            fclose(stream);
            if(src != NULL) {
                fr = eval(self, lk_string_newFromDArray(self, src));
                darray_free(src);
                self->rsrc = self->rsrc->prev;
                return fr;
            } else {
                self->rsrc = self->rsrc->prev;
                lk_vm_raisecstr(self,
                "Cannot read from file named %s", filename);
            }
        } else {
            self->rsrc = self->rsrc->prev;
            lk_vm_raisecstr(self,
            "Cannot open file named %s", filename);
        }
    }
}
lk_scope_t *lk_vm_evalstring(lk_vm_t *self, const char *code) {
    lk_scope_t *fr;
    struct lk_rsrcchain rsrc;
    rsrc.isstring = 1;
    rsrc.rsrc = lk_string_newFromCString(self, code);
    rsrc.prev = self->rsrc;
    self->rsrc = &rsrc;
    fr = eval(self, rsrc.rsrc);
    self->rsrc = self->rsrc->prev;
    return fr;
}
#define CALLFUNC(self, func, args) do { \
    (args)->argc = LIST_ISINIT(&(args)->stack) \
    ? LIST_COUNT(&(args)->stack) : 0; \
    if(LK_OBJ_ISCFUNC(LK_OBJ(func))) { \
        LK_CFUNC(func)->func((args)->receiver, (args)); \
        vm->currentScope = (self); \
    } else { \
        (args)->o.parent = LK_OBJ(LK_KFUNC(func)->scope); \
        (args)->self = LK_OBJ_ISSCOPE((args)->receiver) \
        ? LK_KFUNC(func)->scope->self : (args)->receiver; \
        (args)->first = (args)->next = LK_KFUNC(func)->first; \
        (self) = (args); \
    } \
    goto nextinstr; \
} while(0)
void lk_vm_doevalfunc(lk_vm_t *vm) {
    lk_scope_t *self = vm->currentScope;
    lk_gc_t *gc = vm->gc;
    lk_instr_t *instr;
    lk_scope_t *args;
    /* freq used types */
    lk_object_t *t_func = vm->t_func;
    /* used in slot resolution */
    lk_instr_t *msg;
    lk_string_t *msgn;
    qphash_t *slots;
    setitem_t *si;
    struct lk_slot *slot;
    darray_t *ancs;
    int anci, ancc;
    lk_object_t *recv, *r, *slotv;
    lk_func_t *func;
    /* rescue error and run approp func */
    struct lk_rescue rescue;
    rescue.prev = vm->rescue;
    rescue.rsrc = vm->rsrc;
    vm->rescue = &rescue;
    if(setjmp(rescue.buf)) {
        recv = LK_OBJ(self->scope);
        args = lk_scope_new(vm);
        lk_scope_stackpush(args, LK_OBJ(vm->lasterror));
        for(; recv != NULL; recv = LK_OBJ(LK_SCOPE(recv)->returnto)) {
            if((slots = recv->o.slots) == NULL) continue;
            if((si = qphash_get(slots, vm->str_rescue)) == NULL) continue;
            slot = LK_SLOT(SETITEM_VALUEPTR(si));
            slotv = lk_object_getvaluefromslot(recv, slot);
            if(!LK_OBJ_ISFUNC(slot->check)
            || LK_OBJ_ISA(slotv, t_func) < 3) continue;
            func = lk_func_match(LK_FUNC(slotv), args, args->self);
            if(func == NULL) continue;
            args->receiver = recv;
            args->returnto = LK_SCOPE(recv)->returnto;
            args->func = slotv; /* LK_OBJ(func); */
            CALLFUNC(self, func, args);
        }
        vm->rescue = vm->rescue->prev;
        vm->rsrc = vm->rescue != NULL ? vm->rescue->rsrc : NULL;
        lk_vm_raiseerror(vm, vm->lasterror);
    }
    nextinstr:
    /* gc memory? sweep triggered by mark if necessary */
    if(gc->newvalues > 5000) {
        lk_gc_mark(gc);
        gc->newvalues = 0;
    }
    /* like how cpu execs instrs by following a program sizeer */
    if((instr = self->next) == NULL) goto prevscope;
    vm->stat.totalInstructions ++;
    vm->currinstr = self->current = instr;
    self->next = instr->next;
    switch(instr->type) {
    /* skip comments */
    /* msg represents a possiblity, a function to be exec'd */
    case LK_INSTRTYPE_SELFMSG:
        lk_scope_stackpush(self, self->self != NULL ? self->self : NIL);
        goto sendmsg;
    case LK_INSTRTYPE_SCOPEMSG:
        lk_scope_stackpush(self, LK_OBJ(self->scope));
        goto sendmsg;
    case LK_INSTRTYPE_APPLYMSG:
        sendmsg:
        lk_scope_stackpush(self, LK_OBJ(instr));
        if(instr->opts & LK_INSTROHASMSGARGS) goto nextinstr;
        args = NULL;
        goto apply;
    case LK_INSTRTYPE_APPLY:
    case LK_INSTRTYPE_LIST:
        args = lk_scope_new(vm);
        args->first = args->next = LK_INSTR(instr->v);
        args->type = instr->type == LK_INSTRTYPE_APPLY
        ? LK_SCOPETYPE_APPLY : LK_SCOPETYPE_LIST;
        args->scope = self->scope;
        args->receiver = self->receiver;
        self = args;
        goto nextinstr;
    /* "literals" are actually generators */
    case LK_INSTRTYPE_NUMBER:
    case LK_INSTRTYPE_STRING: 
    case LK_INSTRTYPE_CHAR: 
        lk_scope_stackpush(self, lk_object_clone(instr->v));
        goto nextinstr;
    /* funcs also need ref to env for closures to work */
    case LK_INSTRTYPE_FUNC: {
        lk_kfunc_t *clone = LK_KFUNC(lk_object_clone(instr->v));
        lk_kfunc_updatesig(clone);
        lk_object_addref(LK_OBJ(clone), LK_OBJ(self->scope));
        clone->scope = self->scope;
        lk_object_addref(LK_OBJ(clone), LK_OBJ(self->receiver));
        lk_scope_stackpush(self, LK_OBJ(clone));
        goto nextinstr; }
    /* read more instr from the parser */
    case LK_INSTRTYPE_MORE:
        self->next = lk_parser_getmore(LK_PARSER(instr->v));
        goto nextinstr;
    /* should never happen */
    default: BUG("Invalid instruction type");
    }
    /* return from func */
    prevscope:
    args = self;
    self = self->returnto;
    if(self == NULL) {
        vm->rescue = vm->rescue->prev;
        vm->currentScope = vm->currentScope->caller;
        return;
    }
    switch(args->type) {
    /* take the scope and return last val */
    case LK_SCOPETYPE_RETURN:
        lk_scope_stackpush(self, lk_scope_stackpeek(args));
        vm->currentScope = self;
        goto nextinstr;
    /* take the scope and convert to list */
    case LK_SCOPETYPE_LIST:
        lk_scope_stackpush(self, LK_OBJ(lk_scope_stacktolist(args)));
        vm->currentScope = self;
        goto nextinstr;
    /* take the scope and use as args for msg */
    case LK_SCOPETYPE_APPLY:
        apply:
        /*
        msg = LK_INSTR(lk_scope_stackpop(self));
        msgn = LK_STRING(msg->v);
        recv = r = lk_scope_stackpop(self);
        */
        recv = r = lk_scope_stackpop(self);
        if(LK_OBJ_ISINSTR(recv)) {
            msg = LK_INSTR(recv);
            msgn = LK_STRING(msg->v);
            recv = r = lk_scope_stackpop(self);
        } else {
            msg = NULL;
            msgn = vm->str_at;
        }
        ancs = NULL;
        findslot:
        if((slots = r->o.slots) == NULL) goto parent;
        if((si = qphash_get(slots, msgn)) == NULL) goto parent;
        found:
        slot = LK_SLOT(SETITEM_VALUEPTR(si));
        slotv = lk_object_getvaluefromslot(recv, slot);
        /* slot contains func obj - call? */
        if(LK_OBJ_ISA(slotv, t_func) > 2
        && LK_SLOT_CHECKOPTION(slot, LK_SLOTOPTION_AUTOSEND)
        && (instr == NULL
        || instr->next == NULL
        || instr->next->type != LK_INSTRTYPE_APPLYMSG
        || darray_compareToCString(DARRAY(instr->next->v), "+=") != 0)) {
            callfunc:
            if(args == NULL) args = lk_scope_new(vm);
            func = lk_func_match(LK_FUNC(slotv), args, recv);
            if(func == NULL) goto parent;
            args->type = LK_SCOPETYPE_RETURN;
            args->scope = args;
            args->receiver = recv;
            args->func = slotv; /* LK_OBJ(func); */
            CALLFUNC(self, func, args);
        } else {
            /* called like a func? */
            if(args != NULL) {
                /* slot contains func */
                if(LK_OBJ_ISA(slotv, t_func) > 2) {
                    goto callfunc;
                /* call at/apply if there are args */
                } else if(LIST_ISINIT(&args->stack)
                       && LIST_COUNT(&args->stack) > 0) {
                    msgn = vm->str_at;
                    recv = r = slotv;
                    ancs = NULL;
                    goto findslot;
                }
            }
            self->lastslot = slot;
            lk_scope_stackpush(self, slotv);
            goto nextinstr;
        }
        parent:
        if((ancs = r->o.ancestors) != NULL) {
            ancc = LIST_COUNT(ancs);
            for(anci = 1; anci < ancc; anci ++) {
                r = LIST_ATPTR(ancs, anci);
                if((slots = r->o.slots) == NULL) continue;
                if((si = qphash_get(slots, msg->v)) == NULL) continue;
                goto found;
            }
        } else {
            r = r->o.parent;
            goto findslot;
        }
        /* forward: */
        if(LIST_EQ(DARRAY(msgn), DARRAY(vm->str_forward))) {
            lk_vm_raisecstr(vm, "Cannot find slot named %s", msg->v);
        } else {
            msgn = vm->str_forward;
            r = recv;
            ancs = NULL;
            goto findslot;
        }
    /* should never happen */
    default: BUG("Invalid scope type");
    }
}
void lk_vm_raisecstr(lk_vm_t *self, const char *message, ...) {
    lk_error_t *error = LK_ERROR(lk_object_alloc(self->t_error));
    va_list ap;
    error->instr = self->currinstr;
    error->message = LK_STRING(lk_object_alloc(self->t_string));
    va_start(ap, message);
    for(; *message != '\0'; message ++) {
        if(*message == '%') {
            message ++;
            switch(*message) {
            case 's':
                darray_concat(DARRAY(error->message), DARRAY(va_arg(ap, lk_string_t *)));
                break;
            }
        } else {
            darray_pushuchar(DARRAY(error->message), *message);
        }
    }
    va_end(ap);
    lk_vm_raiseerror(self, error);
}
void lk_vm_raiseerrno(lk_vm_t *self) {
    lk_error_t *error = LK_ERROR(lk_object_alloc(self->t_error));
    error->message = lk_string_newFromCString(self, strerror(errno));
    lk_vm_raiseerror(self, error);
}
void lk_vm_raiseerror(lk_vm_t *self, lk_error_t *error) {
    if(self->rescue == NULL) lk_vm_abort(self, error);
    else {
        self->lasterror = error;
        longjmp(self->rescue->buf, 1);
    }
}
void lk_vm_exit(lk_vm_t *self) {
    lk_vm_free(self);
    exit(EXIT_SUCCESS);
}
void lk_vm_abort(lk_vm_t *self, lk_error_t *error) {
    if(error != NULL) {
        struct lk_slot *slot = lk_object_getslotfromany(
        LK_OBJ(error), LK_OBJ(self->str_type));
        lk_string_t *type = LK_STRING(lk_object_getvaluefromslot(LK_OBJ(error), slot));
        lk_instr_t *expr = error->instr;
        int i = 0;
        darray_printToStream(DARRAY(type), stdout);
        fprintf(stdout, "\n* rsrc: ");
        darray_printToStream(DARRAY(expr->rsrc), stdout);
        fprintf(stdout, "\n* line: %i", expr->line);
        while(expr->prev != NULL) { expr = expr->prev; i ++; }
        fprintf(stdout, "\n* instruction(%i): ", i);
        lk_instr_print(expr);
        fprintf(stdout, "\n* text: ");
        if(error->message != NULL) darray_printToStream(DARRAY(error->message), stdout);
        printf("\n");
    } else {
        printf("Unknown error!\n");
    }
    lk_vm_free(self);
    exit(EXIT_FAILURE);
}
