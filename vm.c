#include "vm.h"
#include "bool.h"
#include "char.h"
#include "cset.h"
#include "env.h"
#include "error.h"
#include "ext.h"
#include "file.h"
#include "vector.h"
#include "fixnum.h"
#include "frame.h"
#include "func.h"
#include "gc.h"
#include "glist.h"
#include "gset.h"
#include "instr.h"
#include "list.h"
#include "map.h"
#include "obj.h"
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
LK_EXT_DEFINIT(lk_Vm_extinittypes) {
    vm->t_vm = lk_Object_alloc(vm->t_obj);
}

/* ext map - funcs */
LK_LIBRARY_DEFINECFUNCTION(exit__vm) {
    lk_Vm_exit(VM);
    DONE;
}
LK_LIBRARY_DEFINECFUNCTION(fork__vm) {
    pid_t child = fork();
    if(child == -1) lk_Vm_raiseerrno(VM);
    RETURN(lk_Fi_new(VM, (int)child));
}
LK_LIBRARY_DEFINECFUNCTION(fork__vm_f) {
    pid_t child = fork();
    if(child == -1) lk_Vm_raiseerrno(VM);
    if(child > 0) RETURN(lk_Fi_new(VM, (int)child));
    else {
        lk_Kfunc_t *kf = LK_KFUNC(ARG(0));
        lk_Frame_t *fr = lk_Frame_new(VM);
        fr->first = fr->next = kf->first;
        fr->receiver = fr->self = self;
        fr->func = LK_OBJ(kf);
        fr->returnto = NULL;
        fr->obj.parent = LK_OBJ(kf->frame);
        lk_Vm_doevalfunc(VM);
        lk_Vm_exit(VM);
        DONE;
    }
}
LK_LIBRARY_DEFINECFUNCTION(sleep_vm_fr) {
    usleep((unsigned long)(DBL(ARG(0)) * 1000000));
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(seconds_since_epoch__vm) {
    struct timeval now;
    gettimeofday(&now, NULL);
    RETURN(lk_Fr_new(VM, now.tv_sec + now.tv_usec / 1000000.0));
}
LK_LIBRARY_DEFINECFUNCTION(seconds_west_of_utc__vm) {
    struct timeval now;
    time_t t;
    struct tm gm, local;
    gettimeofday(&now, NULL);
    t = now.tv_sec;
    gm = *gmtime(&t);
    local = *localtime(&t);
    RETURN(lk_Fr_new(VM, (local.tm_sec - gm.tm_sec
    + (local.tm_min - gm.tm_min) * 60
    + (local.tm_hour - gm.tm_hour) * 3600)
    - (local.tm_year < gm.tm_year
    || local.tm_mon < gm.tm_mon
    || local.tm_mday < gm.tm_mday ? 86400 : 0)));
}
LK_LIBRARY_DEFINECFUNCTION(system__vm) {
    pid_t child = fork();
    if(child == -1) lk_Vm_raiseerrno(VM);
    if(child > 0) {
        int status;
        waitpid(child, &status, 0);
        RETURN(lk_Fi_new(VM, WEXITSTATUS(status)));
    } else {
        int i, c = env->argc;
        char **args = memory_alloc(sizeof(char *) * (c + 1));
        for(i = 0; i < c; i ++) {
            args[i] = (char *)Sequence_tocstr(LIST(ARG(i)));
        }
        execvp(args[0], args);
        lk_Vm_raiseerrno(VM);
    }
}
LK_LIBRARY_DEFINECFUNCTION(system2__vm_str) {
    FILE *out = popen(Sequence_tocstr(LIST(ARG(0))), "r");
    if(out != NULL) {
        char ret[4096];
        char *line = fgets(ret, 4096, out);
        if(line != NULL) RETURN(lk_String_newfromcstr(VM, line));
    }
    RETURN(N);
}
LK_LIBRARY_DEFINECFUNCTION(wait__vm) {
    int status;
    pid_t child = wait(&status);
    RETURN(lk_Fi_new(VM, (int)child));
}
LK_EXT_DEFINIT(lk_Vm_extinitfuncs) {
    lk_Object_t *tvm = vm->t_vm, *f = vm->t_func, *fr = vm->t_fr,
                *str = vm->t_string;
    lk_Library_setGlobal("VirtualMachine", tvm);
    lk_Library_setCFunction(tvm, "exit", exit__vm, NULL);
    lk_Library_setCFunction(tvm, "fork", fork__vm, NULL);
    lk_Library_setCFunction(tvm, "fork", fork__vm_f, f, NULL);
    lk_Library_setCFunction(tvm, "sleep", sleep_vm_fr, fr, NULL);
    lk_Library_setCFunction(tvm, "seconds since epoch", seconds_since_epoch__vm, NULL);
    lk_Library_setCFunction(tvm, "seconds west of utc", seconds_west_of_utc__vm, NULL);
    lk_Library_setCFunction(tvm, "system", system__vm, -1);
    lk_Library_setCFunction(tvm, "system2", system2__vm_str, str, NULL);
    lk_Library_setCFunction(tvm, "wait", wait__vm, NULL);
}

/* new */
lk_Vm_t *lk_Vm_new(void) {
    lk_Vm_t *self = memory_alloc(sizeof(lk_Vm_t));

    /* must be loaded before other primitive types */
    lk_Object_extinittypes(self);
    lk_Gc_extinittypes(self);
    lk_Vm_extinittypes(self);
    lk_GSequence_extinittypes(self);
    lk_Gset_extinittypes(self);
    lk_String_extinittypes(self);

    /* init all other primitive types */
    lk_Boolean_extinittypes(self);
    lk_Char_extinittypes(self);
    lk_Cset_extinittypes(self);
    lk_Map_extinittypes(self);
    lk_Error_extinittypes(self);
    lk_File_extinittypes(self);
    lk_Folder_extinittypes(self);
    lk_Vector_extinittypes(self);
    lk_Fixnum_extinittypes(self);
    lk_Frame_extinittypes(self);
    lk_Func_extinittypes(self);
    lk_Instr_extinittypes(self);
    lk_List_extinittypes(self);
    lk_Parser_extinittypes(self);

    /* init rest of the fields in vm */
    self->global = LK_FRAME(lk_Object_alloc(self->t_frame));
    self->currentFrame = self->global;
    lk_Library_setGlobal(".string.class", LK_OBJ(self->str_type = lk_String_newfromcstr(self, ".CLASS")));
    lk_Library_setGlobal(".string.forward", LK_OBJ(self->str_forward = lk_String_newfromcstr(self, ".forward")));
    lk_Library_setGlobal(".string.rescue", LK_OBJ(self->str_rescue = lk_String_newfromcstr(self, ".rescue")));
    lk_Library_setGlobal(".string.on assign", LK_OBJ(self->str_onassign = lk_String_newfromcstr(self, ".on_assign")));
    lk_Library_setGlobal(".string.at", LK_OBJ(self->str_at = lk_String_newfromcstr(self, "at")));
    lk_Library_setGlobal(".string.slash", LK_OBJ(self->str_filesep = lk_String_newfromcstr(self, "/")));

    /* attach all funcs to primitive types */
    lk_Boolean_extinitfuncs(self);
    lk_Char_extinitfuncs(self);
    lk_Cset_extinitfuncs(self);
    lk_Map_extinitfuncs(self);
    lk_Error_extinitfuncs(self);
    lk_File_extinitfuncs(self);
    lk_Folder_extinitfuncs(self);
    lk_Vector_extinitfuncs(self);
    lk_Fixnum_extinitfuncs(self);
    lk_Frame_extinitfuncs(self);
    lk_Func_extinitfuncs(self);
    lk_Gc_extinitfuncs(self);
    lk_Instr_extinitfuncs(self);
    lk_List_extinitfuncs(self);
    lk_Object_extinitfuncs(self);
    lk_Parser_extinitfuncs(self);
    lk_String_extinitfuncs(self);
    lk_Vm_extinitfuncs(self);
    lk_GSequence_extinitfuncs(self);
    lk_Gset_extinitfuncs(self);

    /* extra libs */
    lk_Env_extinit(self);
    lk_Library_extinit(self);
    lk_Random_extinit(self);
    lk_Socket_extinit(self);
    return self;
}
void lk_Vm_free(lk_Vm_t *self) {
    lk_Gc_t *gc = self->gc;
    lk_Objectgroup_remove(LK_OBJ(gc));
    lk_Objectgroup_freevalues(gc->unused);
    lk_Objectgroup_freevalues(gc->pending);
    lk_Objectgroup_freevalues(gc->used);
    lk_Object_justfree(LK_OBJ(gc));
    memory_free(self);

    /*
    fprintf(stderr, "alloccount: %i\n", memory_alloccount());
    fprintf(stderr, "alloctotal: %i\n", memory_alloctotal());
    fprintf(stderr, "allocpeak: %i\n", memory_allocpeak());
    fprintf(stderr, "allocused: %i\n", memory_allocused());
    */
}

/* eval */
static lk_Frame_t *eval(lk_Vm_t *self, lk_String_t *code) {
    lk_Parser_t *p = lk_Parser_new(self);
    lk_Instr_t *func = lk_Parser_parse(p, code);
    lk_Frame_t *fr = lk_Frame_new(self);
    fr->first = fr->next = func;
    fr->returnto = NULL;
    lk_Vm_doevalfunc(self);
    return fr;
}
lk_Frame_t *lk_Vm_evalfile(lk_Vm_t *self, const char *file, const char *base) {
    lk_String_t *filename = lk_String_newfromcstr(self, file);
    if(base != NULL) {
        lk_String_t *root = lk_String_newfromcstr(self, base);
        int pos, i, pslen = LIST_COUNT(LIST(self->str_filesep));
        pos = Sequence_findlist(LIST(root), LIST(self->str_filesep), 0);
        if(pos > 0) {
            lk_String_t *orig = filename;
            root = lk_String_newfromlist(self, LIST(root));
            pos += pslen;
            while((i = Sequence_findlist(
            LIST(root), LIST(self->str_filesep), pos)) > 0) pos = i + pslen;
            Sequence_slice(LIST(root), 0, pos);
            Sequence_resizeitem(LIST(root), LIST(orig));
            Sequence_concat(LIST(root), LIST(orig));
            filename = root;
        }
    }
    {
        lk_Frame_t *fr;
        struct lk_Rsrcchain rsrc;
        FILE *stream;
        const char *cfilename = Sequence_tocstr(LIST(filename));
        rsrc.isstring = 0;
        rsrc.rsrc = filename;
        rsrc.prev = self->rsrc;
        self->rsrc = &rsrc;
        stream = fopen(cfilename, "r");
        if(stream != NULL) {
            Sequence_t *src = string_allocfromfile(stream);
            fclose(stream);
            if(src != NULL) {
                fr = eval(self, lk_String_newfromlist(self, src));
                Sequence_free(src);
                self->rsrc = self->rsrc->prev;
                return fr;
            } else {
                self->rsrc = self->rsrc->prev;
                lk_Vm_raisecstr(self,
                "Cannot read from file named %s", filename);
            }
        } else {
            self->rsrc = self->rsrc->prev;
            lk_Vm_raisecstr(self,
            "Cannot open file named %s", filename);
        }
    }
}
lk_Frame_t *lk_Vm_evalstring(lk_Vm_t *self, const char *code) {
    lk_Frame_t *fr;
    struct lk_Rsrcchain rsrc;
    rsrc.isstring = 1;
    rsrc.rsrc = lk_String_newfromcstr(self, code);
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
        vm->currentFrame = (self); \
    } else { \
        (args)->obj.parent = LK_OBJ(LK_KFUNC(func)->frame); \
        (args)->self = LK_OBJ_ISFRAME((args)->receiver) \
        ? LK_KFUNC(func)->frame->self : (args)->receiver; \
        (args)->first = (args)->next = LK_KFUNC(func)->first; \
        (self) = (args); \
    } \
    goto nextinstr; \
} while(0)
void lk_Vm_doevalfunc(lk_Vm_t *vm) {
    lk_Frame_t *self = vm->currentFrame;
    lk_Gc_t *gc = vm->gc;
    lk_Instr_t *instr;
    lk_Frame_t *args;
    /* freq used types */
    lk_Object_t *t_func = vm->t_func;
    /* used in slot resolution */
    lk_Instr_t *msg;
    lk_String_t *msgn;
    set_t *slots;
    setitem_t *si;
    struct lk_Slot *slot;
    Sequence_t *ancs;
    int anci, ancc;
    lk_Object_t *recv, *r, *slotv;
    lk_Func_t *func;
    /* rescue error and run approp func */
    struct lk_Rescue rescue;
    rescue.prev = vm->rescue;
    rescue.rsrc = vm->rsrc;
    vm->rescue = &rescue;
    if(setjmp(rescue.buf)) {
        recv = LK_OBJ(self->frame);
        args = lk_Frame_new(vm);
        lk_Frame_stackpush(args, LK_OBJ(vm->lasterror));
        for(; recv != NULL; recv = LK_OBJ(LK_FRAME(recv)->returnto)) {
            if((slots = recv->obj.slots) == NULL) continue;
            if((si = set_get(slots, vm->str_rescue)) == NULL) continue;
            slot = LK_SLOT(SETITEM_VALUEPTR(si));
            slotv = lk_Object_getvaluefromslot(recv, slot);
            if(!LK_OBJ_ISFUNC(slot->check)
            || LK_OBJ_ISA(slotv, t_func) < 3) continue;
            func = lk_Func_match(LK_FUNC(slotv), args, args->self);
            if(func == NULL) continue;
            args->receiver = recv;
            args->returnto = LK_FRAME(recv)->returnto;
            args->func = slotv; /* LK_OBJ(func); */
            CALLFUNC(self, func, args);
        }
        vm->rescue = vm->rescue->prev;
        vm->rsrc = vm->rescue != NULL ? vm->rescue->rsrc : NULL;
        lk_Vm_raiseerror(vm, vm->lasterror);
    }
    nextinstr:
    /* gc memory? sweep triggered by mark if necessary */
    if(gc->newvalues > 5000) {
        lk_Gc_mark(gc);
        gc->newvalues = 0;
    }
    /* like how cpu execs instrs by following a program counter */
    if((instr = self->next) == NULL) goto prevframe;
    vm->stat.totalInstructions ++;
    vm->currinstr = self->current = instr;
    self->next = instr->next;
    switch(instr->type) {
    /* skip comments */
    /* msg represents a possiblity, a function to be exec'd */
    case LK_INSTRTYPE_SELFMSG:
        lk_Frame_stackpush(self, self->self != NULL ? self->self : N);
        goto sendmsg;
    case LK_INSTRTYPE_FRAMEMSG:
        lk_Frame_stackpush(self, LK_OBJ(self->frame));
        goto sendmsg;
    case LK_INSTRTYPE_APPLYMSG:
        sendmsg:
        lk_Frame_stackpush(self, LK_OBJ(instr));
        if(instr->opts & LK_INSTROHASMSGARGS) goto nextinstr;
        args = NULL;
        goto apply;
    case LK_INSTRTYPE_APPLY:
    case LK_INSTRTYPE_LIST:
        args = lk_Frame_new(vm);
        args->first = args->next = LK_INSTR(instr->v);
        args->type = instr->type == LK_INSTRTYPE_APPLY
        ? LK_FRAMETYPE_APPLY : LK_FRAMETYPE_LIST;
        args->frame = self->frame;
        args->receiver = self->receiver;
        self = args;
        goto nextinstr;
    /* "literals" are actually generators */
    case LK_INSTRTYPE_FIXINT:
    case LK_INSTRTYPE_FIXF:
    case LK_INSTRTYPE_STRING: 
    case LK_INSTRTYPE_CHAR: 
        lk_Frame_stackpush(self, lk_Object_clone(instr->v));
        goto nextinstr;
    /* funcs also need ref to env for closures to work */
    case LK_INSTRTYPE_FUNC: {
        lk_Kfunc_t *clone = LK_KFUNC(lk_Object_clone(instr->v));
        lk_Kfunc_updatesig(clone);
        lk_Object_addref(LK_OBJ(clone), LK_OBJ(self->frame));
        clone->frame = self->frame;
        lk_Object_addref(LK_OBJ(clone), LK_OBJ(self->receiver));
        lk_Frame_stackpush(self, LK_OBJ(clone));
        goto nextinstr; }
    /* read more instr from the parser */
    case LK_INSTRTYPE_MORE:
        self->next = lk_Parser_getmore(LK_PARSER(instr->v));
        goto nextinstr;
    /* should never happen */
    default: BUG("Invalid instruction type");
    }
    /* return from func */
    prevframe:
    args = self;
    self = self->returnto;
    if(self == NULL) {
        vm->rescue = vm->rescue->prev;
        vm->currentFrame = vm->currentFrame->caller;
        return;
    }
    switch(args->type) {
    /* take the frame and return last val */
    case LK_FRAMETYPE_RETURN:
        lk_Frame_stackpush(self, lk_Frame_stackpeek(args));
        vm->currentFrame = self;
        goto nextinstr;
    /* take the frame and convert to list */
    case LK_FRAMETYPE_LIST:
        lk_Frame_stackpush(self, LK_OBJ(lk_Frame_stacktolist(args)));
        vm->currentFrame = self;
        goto nextinstr;
    /* take the frame and use as args for msg */
    case LK_FRAMETYPE_APPLY:
        apply:
        /*
        msg = LK_INSTR(lk_Frame_stackpop(self));
        msgn = LK_STRING(msg->v);
        recv = r = lk_Frame_stackpop(self);
        */
        recv = r = lk_Frame_stackpop(self);
        if(LK_OBJ_ISINSTR(recv)) {
            msg = LK_INSTR(recv);
            msgn = LK_STRING(msg->v);
            recv = r = lk_Frame_stackpop(self);
        } else {
            msg = NULL;
            msgn = vm->str_at;
        }
        ancs = NULL;
        findslot:
        if((slots = r->obj.slots) == NULL) goto parent;
        if((si = set_get(slots, msgn)) == NULL) goto parent;
        found:
        slot = LK_SLOT(SETITEM_VALUEPTR(si));
        slotv = lk_Object_getvaluefromslot(recv, slot);
        /* slot contains func obj - call? */
        if(LK_OBJ_ISA(slotv, t_func) > 2
        && LK_SLOT_CHECKOPTION(slot, LK_SLOTOPTION_AUTOSEND)
        && (instr == NULL
        || instr->next == NULL
        || instr->next->type != LK_INSTRTYPE_APPLYMSG
        || Sequence_cmpcstr(LIST(instr->next->v), "+=") != 0)) {
            callfunc:
            if(args == NULL) args = lk_Frame_new(vm);
            func = lk_Func_match(LK_FUNC(slotv), args, recv);
            if(func == NULL) goto parent;
            args->type = LK_FRAMETYPE_RETURN;
            args->frame = args;
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
            lk_Frame_stackpush(self, slotv);
            goto nextinstr;
        }
        parent:
        if((ancs = r->obj.ancestors) != NULL) {
            ancc = LIST_COUNT(ancs);
            for(anci = 1; anci < ancc; anci ++) {
                r = LIST_ATPTR(ancs, anci);
                if((slots = r->obj.slots) == NULL) continue;
                if((si = set_get(slots, msg->v)) == NULL) continue;
                goto found;
            }
        } else {
            r = r->obj.parent;
            goto findslot;
        }
        /* forward: */
        if(LIST_EQ(LIST(msgn), LIST(vm->str_forward))) {
            lk_Vm_raisecstr(vm, "Cannot find slot named %s", msg->v);
        } else {
            msgn = vm->str_forward;
            r = recv;
            ancs = NULL;
            goto findslot;
        }
    /* should never happen */
    default: BUG("Invalid frame type");
    }
}
void lk_Vm_raisecstr(lk_Vm_t *self, const char *message, ...) {
    lk_Error_t *error = LK_ERROR(lk_Object_alloc(self->t_error));
    va_list ap;
    error->instr = self->currinstr;
    error->text = LK_STRING(lk_Object_alloc(self->t_string));
    va_start(ap, message);
    for(; *message != '\0'; message ++) {
        if(*message == '%') {
            message ++;
            switch(*message) {
            case 's':
                Sequence_concat(LIST(error->text), LIST(va_arg(ap, lk_String_t *)));
                break;
            }
        } else {
            Sequence_pushuchar(LIST(error->text), *message);
        }
    }
    va_end(ap);
    lk_Vm_raiseerror(self, error);
}
void lk_Vm_raiseerrno(lk_Vm_t *self) {
    lk_Error_t *error = LK_ERROR(lk_Object_alloc(self->t_error));
    error->text = lk_String_newfromcstr(self, strerror(errno));
    lk_Vm_raiseerror(self, error);
}
void lk_Vm_raiseerror(lk_Vm_t *self, lk_Error_t *error) {
    if(self->rescue == NULL) lk_Vm_abort(self, error);
    else {
        self->lasterror = error;
        longjmp(self->rescue->buf, 1);
    }
}
void lk_Vm_exit(lk_Vm_t *self) {
    lk_Vm_free(self);
    exit(EXIT_SUCCESS);
}
void lk_Vm_abort(lk_Vm_t *self, lk_Error_t *error) {
    if(error != NULL) {
        struct lk_Slot *slot = lk_Object_getslotfromany(
        LK_OBJ(error), LK_OBJ(self->str_type));
        lk_String_t *type = LK_STRING(lk_Object_getvaluefromslot(LK_OBJ(error), slot));
        lk_Instr_t *expr = error->instr;
        int i = 0;
        string_print(LIST(type), stdout);
        fprintf(stdout, "\n* rsrc: ");
        string_print(LIST(expr->rsrc), stdout);
        fprintf(stdout, "\n* line: %i", expr->line);
        while(expr->prev != NULL) { expr = expr->prev; i ++; }
        fprintf(stdout, "\n* instruction(%i): ", i);
        lk_Instr_print(expr);
        fprintf(stdout, "\n* text: ");
        if(error->text != NULL) string_print(LIST(error->text), stdout);
        printf("\n");
    } else {
        printf("Unknown error!\n");
    }
    lk_Vm_free(self);
    exit(EXIT_FAILURE);
}
