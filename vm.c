#include "vm.h"
#include "bool.h"
#include "buffer.h"
#include "char.h"
#include "cset.h"
#include "dict.h"
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
LK_EXT_DEFINIT(lk_vm_extinittypes) {
    vm->t_vm = lk_object_alloc(vm->t_object);
}

/* ext map - funcs */
static LK_EXT_DEFCFUNC(exit__vm) {
    lk_vm_exit(VM);
    DONE;
}
static LK_EXT_DEFCFUNC(fork__vm) {
    pid_t child = fork();
    if(child == -1) lk_vm_raiseerrno(VM);
    RETURN(lk_fi_new(VM, (int)child));
}
static LK_EXT_DEFCFUNC(fork__vm_f) {
    pid_t child = fork();
    if(child == -1) lk_vm_raiseerrno(VM);
    if(child > 0) RETURN(lk_fi_new(VM, (int)child));
    else {
        lk_kfunc_t *kf = LK_KFUNC(ARG(0));
        lk_frame_t *fr = lk_vm_prepevalfunc(VM);
        fr->first = fr->next = kf->first;
        fr->receiver = fr->self = self;
        fr->func = LK_O(kf);
        fr->returnto = NULL;
        fr->co.proto = LK_O(kf->frame);
        lk_vm_doevalfunc(VM);
        lk_vm_exit(VM);
        DONE;
    }
}
static LK_EXT_DEFCFUNC(sleep_vm_fr) {
    usleep((unsigned long)(DBL(ARG(0)) * 1000000));
    RETURN(self);
}
static LK_EXT_DEFCFUNC(seconds_since_epoch__vm) {
    struct timeval now;
    gettimeofday(&now, NULL);
    RETURN(lk_fr_new(VM, now.tv_sec + now.tv_usec / 1000000.0));
}
static LK_EXT_DEFCFUNC(seconds_west_of_utc__vm) {
    struct timeval now;
    time_t t;
    struct tm gm, local;
    gettimeofday(&now, NULL);
    t = now.tv_sec;
    gm = *gmtime(&t);
    local = *localtime(&t);
    RETURN(lk_fr_new(VM, (local.tm_sec - gm.tm_sec
    + (local.tm_min - gm.tm_min) * 60
    + (local.tm_hour - gm.tm_hour) * 3600)
    - (local.tm_year < gm.tm_year
    || local.tm_mon < gm.tm_mon
    || local.tm_mday < gm.tm_mday ? 86400 : 0)));
}
static LK_EXT_DEFCFUNC(system__vm) {
    pid_t child = fork();
    if(child == -1) lk_vm_raiseerrno(VM);
    if(child > 0) {
        int status;
        waitpid(child, &status, 0);
        RETURN(lk_fi_new(VM, WEXITSTATUS(status)));
    } else {
        int i, c = env->argc;
        char **args = memory_alloc(sizeof(char *) * (c + 1));
        for(i = 0; i < c; i ++) {
            args[i] = (char *)list_tocstr(LIST(ARG(i)));
        }
        execvp(args[0], args);
        lk_vm_raiseerrno(VM);
    }
}
static LK_EXT_DEFCFUNC(system2__vm_str) {
    FILE *out = popen(list_tocstr(LIST(ARG(0))), "r");
    if(out != NULL) {
        char ret[4096];
        char *line = fgets(ret, 4096, out);
        if(line != NULL) RETURN(lk_string_newfromcstr(VM, line));
    }
    RETURN(N);
}
static LK_EXT_DEFCFUNC(wait__vm) {
    int status;
    pid_t child = wait(&status);
    RETURN(lk_fi_new(VM, (int)child));
}
LK_EXT_DEFINIT(lk_vm_extinitfuncs) {
    lk_object_t *tvm = vm->t_vm, *f = vm->t_func, *fr = vm->t_fr,
                *str = vm->t_string;
    lk_ext_global("VirtualMachine", tvm);
    lk_ext_cfunc(tvm, "exit", exit__vm, NULL);
    lk_ext_cfunc(tvm, "fork", fork__vm, NULL);
    lk_ext_cfunc(tvm, "fork", fork__vm_f, f, NULL);
    lk_ext_cfunc(tvm, "sleep", sleep_vm_fr, fr, NULL);
    lk_ext_cfunc(tvm, "seconds since epoch", seconds_since_epoch__vm, NULL);
    lk_ext_cfunc(tvm, "seconds west of utc", seconds_west_of_utc__vm, NULL);
    lk_ext_cfunc(tvm, "system", system__vm, -1);
    lk_ext_cfunc(tvm, "system2", system2__vm_str, str, NULL);
    lk_ext_cfunc(tvm, "wait", wait__vm, NULL);
}

/* new */
lk_vm_t *lk_vm_new(void) {
    static int vm_id = 0;
    return lk_vm_newwithid(vm_id ++);
}
lk_vm_t *lk_vm_newwithid(int id) {
    lk_vm_t *self = memory_alloc(sizeof(lk_vm_t));
    self->id = id;
    self->retained = list_allocptr();
    self->symbols = set_alloc(0, lk_object_hashcode, lk_object_keycmp);

    /* must be loaded before other primitive types */
    lk_object_extinittypes(self);
    lk_gc_extinittypes(self);
    lk_vm_extinittypes(self);
    lk_glist_extinittypes(self);
    lk_gset_extinittypes(self);
    lk_string_extinittypes(self);

    /* init all other primitive types */
    lk_bool_extinittypes(self);
    lk_buffer_extinittypes(self);
    lk_char_extinittypes(self);
    lk_cset_extinittypes(self);
    lk_dict_extinittypes(self);
    lk_error_extinittypes(self);
    lk_file_extinittypes(self);
    lk_vector_extinittypes(self);
    lk_fixnum_extinittypes(self);
    lk_frame_extinittypes(self);
    lk_func_extinittypes(self);
    lk_instr_extinittypes(self);
    lk_list_extinittypes(self);
    lk_parser_extinittypes(self);

    /* init rest of the fields in vm */
    self->global = LK_FRAME(lk_object_alloc(self->t_frame));
    self->currframe = self->global;
    self->str_type = lk_string_newfromcstr(self, ".type");
    self->str_forward = lk_string_newfromcstr(self, ".forward");
    self->str_rescue = lk_string_newfromcstr(self, ".rescue");
    self->str_onassign = lk_string_newfromcstr(self, ".on_assign");
    self->str_at = lk_string_newfromcstr(self, "at");
    self->str_filesep = lk_string_newfromcstr(self, "/");
    list_pushptr(self->retained, self->str_type);
    list_pushptr(self->retained, self->str_forward);
    list_pushptr(self->retained, self->str_rescue);
    list_pushptr(self->retained, self->str_onassign);
    list_pushptr(self->retained, self->str_at);
    list_pushptr(self->retained, self->str_filesep);

    /* attach all funcs to primitive types */
    lk_bool_extinitfuncs(self);
    lk_buffer_extinitfuncs(self);
    lk_char_extinitfuncs(self);
    lk_cset_extinitfuncs(self);
    lk_dict_extinitfuncs(self);
    lk_error_extinitfuncs(self);
    lk_file_extinitfuncs(self);
    lk_vector_extinitfuncs(self);
    lk_fixnum_extinitfuncs(self);
    lk_frame_extinitfuncs(self);
    lk_func_extinitfuncs(self);
    lk_gc_extinitfuncs(self);
    lk_instr_extinitfuncs(self);
    lk_list_extinitfuncs(self);
    lk_object_extinitfuncs(self);
    lk_parser_extinitfuncs(self);
    lk_string_extinitfuncs(self);
    lk_vm_extinitfuncs(self);
    lk_glist_extinitfuncs(self);
    lk_gset_extinitfuncs(self);

    /* extra libs */
    lk_env_extinit(self);
    lk_ext_extinit(self);
    lk_random_extinit(self);
    lk_socket_extinit(self);
    return self;
}
void lk_vm_free(lk_vm_t *self) {
    lk_gc_t *gc = self->gc;
    lk_objgroup_remove(LK_O(gc));
    lk_objgroup_freevalues(gc->unused);
    lk_objgroup_freevalues(gc->pending);
    lk_objgroup_freevalues(gc->used);
    lk_object_justfree(LK_O(gc));
    memory_free(self);

    /*
    fprintf(stderr, "alloctotal: %i\n", memory_alloctotal());
    fprintf(stderr, "allocpeak: %i\n", memory_allocpeak());
    fprintf(stderr, "allocused: %i\n", memory_allocused());
     */

    memory_freerecycled();
}

/* eval */
static lk_frame_t *eval(lk_vm_t *self, lk_string_t *code) {
    lk_parser_t *p = lk_parser_new(self);
    lk_instr_t *func = lk_parser_parse(p, code);
    lk_frame_t *fr = lk_vm_prepevalfunc(self);
    fr->first = fr->next = func;
    fr->returnto = NULL;
    lk_vm_doevalfunc(self);
    return fr;
}
lk_frame_t *lk_vm_evalfile(lk_vm_t *self, const char *file, const char *base) {
    lk_string_t *filename = lk_string_newfromcstr(self, file);
    if(base != NULL) {
        lk_string_t *root = lk_string_newfromcstr(self, base);
        int pos, i, pslen = LIST_COUNT(LIST(self->str_filesep));
        pos = list_findlist(LIST(root), LIST(self->str_filesep), 0);
        if(pos > 0) {
            lk_string_t *orig = filename;
            root = lk_string_newfromlist(self, LIST(root));
            pos += pslen;
            while((i = list_findlist(
            LIST(root), LIST(self->str_filesep), pos)) > 0) pos = i + pslen;
            list_slice(LIST(root), 0, pos);
            list_resizeitem(LIST(root), LIST(orig));
            list_concat(LIST(root), LIST(orig));
            filename = root;
        }
    }
    {
        lk_frame_t *fr;
        struct lk_rsrcchain rsrc;
        FILE *stream;
        const char *cfilename = list_tocstr(LIST(filename));
        rsrc.isstring = 0;
        rsrc.rsrc = filename;
        rsrc.prev = self->rsrc;
        self->rsrc = &rsrc;
        stream = fopen(cfilename, "r");
        if(stream != NULL) {
            list_t *src = string_allocfromfile(stream);
            if(src != NULL) {
                fr = eval(self, lk_string_newfromlist(self, src));
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
lk_frame_t *lk_vm_evalstring(lk_vm_t *self, const char *code) {
    lk_frame_t *fr;
    struct lk_rsrcchain rsrc;
    rsrc.isstring = 1;
    rsrc.rsrc = lk_string_newfromcstr(self, code);
    rsrc.prev = self->rsrc;
    self->rsrc = &rsrc;
    fr = eval(self, rsrc.rsrc);
    self->rsrc = self->rsrc->prev;
    return fr;
}
lk_frame_t *lk_vm_prepevalfunc(lk_vm_t *vm) {
    lk_frame_t *prev = vm->currframe;
    lk_frame_t *self;
    if(prev->child != NULL && prev->child->co.mark.isref == 0) {
        self = prev->child;
        self->co.proto = LK_O(prev);
        if(self->co.slots != NULL) set_clear(self->co.slots);
        list_clear(&self->stack);
        /* stat */ vm->cnt_recycledframe ++;
    } else {
        self = prev->child = LK_FRAME(lk_object_alloc(LK_O(prev)));
    }
    vm->currframe = self;
    self->type = LK_FRAMETYPE_RETURN;
    self->frame = self;
    self->receiver = LK_O(self);
    self->self = prev != NULL ? prev->self : NULL;
    self->caller = self->returnto = prev;
    /* stat */ vm->cnt_frame ++;
    return self;
}
#define CALLFUNC(self, func, args) do { \
    (args)->argc = LIST_ISINIT(&(args)->stack) \
    ? LIST_COUNT(&(args)->stack) : 0; \
    if(LK_OBJECT_ISCFUNC(LK_O(func))) { \
        LK_CFUNC(func)->func((args)->receiver, (args)); \
        vm->currframe = (self); \
    } else { \
        (args)->co.proto = LK_O(LK_KFUNC(func)->frame); \
        (args)->self = LK_OBJECT_ISFRAME((args)->receiver) \
        ? LK_KFUNC(func)->frame->self : (args)->receiver; \
        (args)->first = (args)->next = LK_KFUNC(func)->first; \
        (self) = (args); \
    } \
    goto nextinstr; \
} while(0)
void lk_vm_doevalfunc(lk_vm_t *vm) {
    lk_frame_t *self = vm->currframe;
    lk_gc_t *gc = vm->gc;
    lk_instr_t *instr;
    lk_frame_t *args;
    /* freq used types */
    lk_object_t *t_func = vm->t_func;
    /* used in slot resolution */
    lk_instr_t *msg;
    lk_string_t *msgn;
    set_t *slots;
    setitem_t *si;
    struct lk_slot *slot;
    list_t *ancs;
    int anci, ancc;
    lk_object_t *recv, *r, *slotv;
    lk_func_t *func;
    /* rescue error and run approp func */
    struct lk_rescue rescue;
    rescue.prev = vm->rescue;
    rescue.rsrc = vm->rsrc;
    vm->rescue = &rescue;
    if(setjmp(rescue.buf)) {
        recv = LK_O(self->frame);
        args = lk_vm_prepevalfunc(vm);
        lk_frame_stackpush(args, LK_O(vm->lasterror));
        for(; recv != NULL; recv = LK_O(LK_FRAME(recv)->returnto)) {
            if((slots = recv->co.slots) == NULL) continue;
            if((si = set_get(slots, vm->str_rescue)) == NULL) continue;
            slot = LK_SLOT(SETITEM_VALUEPTR(si));
            slotv = lk_object_getvaluefromslot(recv, slot);
            if(!LK_OBJECT_ISFUNC(slot->check)
            || LK_OBJECT_ISA(slotv, t_func) < 3) continue;
            func = lk_func_match(LK_FUNC(slotv), args, args->self);
            if(func == NULL) continue;
            args->receiver = recv;
            args->returnto = LK_FRAME(recv)->returnto;
            args->func = slotv; /* LK_O(func); */
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
    /* like how cpu execs instrs by following a program counter */
    if((instr = self->next) == NULL) goto prevframe;
    vm->cnt_instr ++;
    vm->currinstr = self->current = instr;
    self->next = instr->next;
    switch(instr->type) {
    /* skip comments */
    /* msg represents a possiblity, a function to be exec'd */
    case LK_INSTRTYPE_SELFMSG:
        lk_frame_stackpush(self, self->self != NULL ? self->self : N);
        goto sendmsg;
    case LK_INSTRTYPE_FRAMEMSG:
        lk_frame_stackpush(self, LK_O(self->frame));
        goto sendmsg;
    case LK_INSTRTYPE_APPLYMSG:
        sendmsg:
        lk_frame_stackpush(self, LK_O(instr));
        if(instr->opts & LK_INSTROHASMSGARGS) goto nextinstr;
        args = NULL;
        goto apply;
    case LK_INSTRTYPE_APPLY:
    case LK_INSTRTYPE_LIST:
        args = lk_vm_prepevalfunc(vm);
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
        lk_frame_stackpush(self, lk_object_clone(instr->v));
        goto nextinstr;
    /* funcs also need ref to env for closures to work */
    case LK_INSTRTYPE_FUNC: {
        lk_kfunc_t *clone = LK_KFUNC(lk_object_clone(instr->v));
        lk_kfunc_updatesig(clone);
        lk_object_addref(LK_O(clone), LK_O(self->frame));
        clone->frame = self->frame;
        lk_object_addref(LK_O(clone), LK_O(self->receiver));
        lk_frame_stackpush(self, LK_O(clone));
        goto nextinstr; }
    /* read more instr from the parser */
    case LK_INSTRTYPE_MORE:
        self->next = lk_parser_getmore(LK_PARSER(instr->v));
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
        vm->currframe = vm->currframe->caller;
        return;
    }
    switch(args->type) {
    /* take the frame and return last val */
    case LK_FRAMETYPE_RETURN:
        lk_frame_stackpush(self, lk_frame_stackpeek(args));
        vm->currframe = self;
        goto nextinstr;
    /* take the frame and convert to list */
    case LK_FRAMETYPE_LIST:
        lk_frame_stackpush(self, LK_O(lk_frame_stacktolist(args)));
        vm->currframe = self;
        goto nextinstr;
    /* take the frame and use as args for msg */
    case LK_FRAMETYPE_APPLY:
        apply:
        /*
        msg = LK_INSTR(lk_frame_stackpop(self));
        msgn = LK_STRING(msg->v);
        recv = r = lk_frame_stackpop(self);
        */
        recv = r = lk_frame_stackpop(self);
        if(LK_OBJECT_ISINSTR(recv)) {
            msg = LK_INSTR(recv);
            msgn = LK_STRING(msg->v);
            recv = r = lk_frame_stackpop(self);
        } else {
            msg = NULL;
            msgn = vm->str_at;
        }
        ancs = NULL;
        findslot:
        if((slots = r->co.slots) == NULL) goto proto;
        if((si = set_get(slots, msgn)) == NULL) goto proto;
        found:
        slot = LK_SLOT(SETITEM_VALUEPTR(si));
        slotv = lk_object_getvaluefromslot(recv, slot);
        /* slot contains func obj - call? */
        if(LK_OBJECT_ISA(slotv, t_func) > 2
        && LK_SLOT_CHECKOPTION(slot, LK_SLOTOPTION_AUTOSEND)
        && (instr == NULL
        || instr->next == NULL
        || instr->next->type != LK_INSTRTYPE_APPLYMSG
        || list_getuchar(LIST(instr->next->v), -1) != '=')) {
            callfunc:
            if(args == NULL) args = lk_vm_prepevalfunc(vm);
            func = lk_func_match(LK_FUNC(slotv), args, recv);
            if(func == NULL) goto proto;
            args->type = LK_FRAMETYPE_RETURN;
            args->frame = args;
            args->receiver = recv;
            args->func = slotv; /* LK_O(func); */
            CALLFUNC(self, func, args);
        } else {
            /* called like a func? */
            if(args != NULL) {
                /* slot contains func */
                if(LK_OBJECT_ISA(slotv, t_func) > 2) {
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
            lk_frame_stackpush(self, slotv);
            goto nextinstr;
        }
        proto:
        if((ancs = r->co.ancestors) != NULL) {
            ancc = LIST_COUNT(ancs);
            for(anci = 1; anci < ancc; anci ++) {
                r = LIST_ATPTR(ancs, anci);
                if((slots = r->co.slots) == NULL) continue;
                if((si = set_get(slots, msg->v)) == NULL) continue;
                goto found;
            }
        } else {
            r = r->co.proto;
            goto findslot;
        }
        /* forward: */
        if(LIST_EQ(LIST(msgn), LIST(vm->str_forward))) {
            lk_vm_raisecstr(vm, "Cannot find slot named %s", msg->v);
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
void lk_vm_raisecstr(lk_vm_t *self, const char *message, ...) {
    lk_error_t *error = LK_ERROR(lk_object_alloc(self->t_error));
    va_list ap;
    error->instr = self->currinstr;
    error->text = LK_STRING(lk_object_alloc(self->t_string));
    va_start(ap, message);
    for(; *message != '\0'; message ++) {
        if(*message == '%') {
            message ++;
            switch(*message) {
            case 's':
                list_concat(LIST(error->text), LIST(va_arg(ap, lk_string_t *)));
                break;
            }
        } else {
            list_pushuchar(LIST(error->text), *message);
        }
    }
    va_end(ap);
    lk_vm_raiseerror(self, error);
}
void lk_vm_raiseerrno(lk_vm_t *self) {
    lk_error_t *error = LK_ERROR(lk_object_alloc(self->t_error));
    error->text = lk_string_newfromcstr(self, strerror(errno));
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
        LK_O(error), LK_O(self->str_type));
        lk_string_t *type = LK_STRING(lk_object_getvaluefromslot(LK_O(error), slot));
        lk_instr_t *expr = error->instr;
        int i = 0;
        string_print(LIST(type), stdout);
        /*
        fprintf(stdout, "\n* rsrc: ");
        string_print(LIST(expr->rsrc), stdout);
        fprintf(stdout, "\n* line: %i", expr->line);
        while(expr->prev != NULL) { expr = expr->prev; i ++; }
        fprintf(stdout, "\n* instruction(%i): ", i);
        lk_instr_print(expr);
        */
        fprintf(stdout, "\n* text: ");
        if(error->text != NULL) string_print(LIST(error->text), stdout);
        printf("\n");
    } else {
        printf("Unknown error!\n");
    }
    lk_vm_free(self);
    exit(EXIT_FAILURE);
}
