#include "vm.h"
#include "bool.h"
#include "char.h"
#include "charset.h"
#include "env.h"
#include "err.h"
#include "lib.h"
#include "file.h"
#include "vec.h"
#include "num.h"
#include "dir.h"
#include "scope.h"
#include "func.h"
#include "gc.h"
#include "seq.h"
#include "instr.h"
#include "list.h"
#include "map.h"
#include "obj.h"
#include "parser.h"
#include "rand.h"
#include "socket.h"
#include "str.h"
#include <errno.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* ext map - types */
void lk_vm_typeinit(lk_vm_t *vm) {
    vm->t_vm = lk_obj_alloc(vm->t_obj);
}

/* ext map - funcs */
static void exit_vm(lk_obj_t *self, lk_scope_t *local) {
    lk_vm_exit(VM);
    DONE;
}
static void fork_vm(lk_obj_t *self, lk_scope_t *local) {
    pid_t child = fork();
    if(child == -1) lk_vm_raiseerrno(VM);
    RETURN(lk_num_new(VM, (int)child));
}
static void fork_vm_f(lk_obj_t *self, lk_scope_t *local) {
    pid_t child = fork();
    if(child == -1) lk_vm_raiseerrno(VM);
    if(child > 0) RETURN(lk_num_new(VM, (int)child));
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
static void sleep_vm_num(lk_obj_t *self, lk_scope_t *local) {
    usleep((unsigned long)(CNUMBER(ARG(0)) * 1000000));
    RETURN(self);
}
static void seconds_since_epoch_vm(lk_obj_t *self, lk_scope_t *local) {
    struct timeval now;
    gettimeofday(&now, NULL);
    RETURN(lk_num_new(VM, now.tv_sec + now.tv_usec / 1000000.0));
}
static void seconds_west_of_utc_vm(lk_obj_t *self, lk_scope_t *local) {
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
    RETURN(lk_num_new(VM, offset));
}
static void system_vm(lk_obj_t *self, lk_scope_t *local) {
    pid_t child = fork();
    if(child == -1) lk_vm_raiseerrno(VM);
    if(child > 0) {
        int status;
        waitpid(child, &status, 0);
        RETURN(lk_num_new(VM, WEXITSTATUS(status)));
    } else {
        int i, c = local->argc;
        char **args = mem_alloc(sizeof(char *) * (c + 1));
        for(i = 0; i < c; i ++) {
            args[i] = (char *)darray_str_tocstr(DARRAY(ARG(i)));
        }
        execvp(args[0], args);
        lk_vm_raiseerrno(VM);
    }
}
static void system2_vm_str(lk_obj_t *self, lk_scope_t *local) {
    FILE *out = popen(darray_str_tocstr(DARRAY(ARG(0))), "r");
    if(out != NULL) {
        char ret[4096];
        char *line = fgets(ret, 4096, out);
        if(line != NULL) RETURN(lk_str_new_fromcstr(VM, line));
    }
    RETURN(NIL);
}
static void wait_vm(lk_obj_t *self, lk_scope_t *local) {
    int status;
    pid_t child = wait(&status);
    RETURN(lk_num_new(VM, (int)child));
}
void lk_vm_libinit(lk_vm_t *vm) {
    lk_obj_t *tvm = vm->t_vm, *f = vm->t_func, *num = vm->t_num, *str = vm->t_str;
    lk_global_set("VirtualMachine", tvm);
    lk_obj_set_cfunc_lk(tvm, "exit", exit_vm, NULL);
    lk_obj_set_cfunc_lk(tvm, "fork", fork_vm, NULL);
    lk_obj_set_cfunc_lk(tvm, "fork", fork_vm_f, f, NULL);
    lk_obj_set_cfunc_lk(tvm, "sleep", sleep_vm_num, num, NULL);
    lk_obj_set_cfunc_lk(tvm, "seconds since epoch", seconds_since_epoch_vm, NULL);
    lk_obj_set_cfunc_lk(tvm, "seconds west of utc", seconds_west_of_utc_vm, NULL);
    lk_obj_set_cfunc_lk(tvm, "system", system_vm, -1);
    lk_obj_set_cfunc_lk(tvm, "system2", system2_vm_str, str, NULL);
    lk_obj_set_cfunc_lk(tvm, "wait", wait_vm, NULL);
}

/* new */
lk_vm_t *lk_vm_new(void) {
    lk_vm_t *self = mem_alloc(sizeof(lk_vm_t));

    /* must be loaded before other primitive types */
    lk_obj_typeinit(self);
    lk_gc_typeinit(self);
    lk_vm_typeinit(self);
    lk_seq_typeinit(self);
    lk_map_typeinit(self);
    lk_str_typeinit(self);

    /* init all other primitive types */
    lk_bool_typeinit(self);
    lk_char_typeinit(self);
    lk_charset_typeinit(self);
    lk_err_typeinit(self);
    lk_file_typeinit(self);
    lk_dir_typeinit(self);
    lk_vec_typeinit(self);
    lk_num_typeinit(self);
    lk_scope_typeinit(self);
    lk_func_typeinit(self);
    lk_instr_typeinit(self);
    lk_list_typeinit(self);
    lk_parser_typeinit(self);
    lk_pipe_typeinit(self);
    lk_dl_typeinit(self);

    /* init rest of the fields in vm */
    self->global = LK_SCOPE(lk_obj_alloc(self->t_scope));
    self->currscope = self->global;
    lk_global_set(".str.class", LK_OBJ(self->str_type = lk_str_new_fromcstr(self, ".CLASS")));
    lk_global_set(".str.forward", LK_OBJ(self->str_forward = lk_str_new_fromcstr(self, ".forward")));
    lk_global_set(".str.rescue", LK_OBJ(self->str_rescue = lk_str_new_fromcstr(self, ".rescue")));
    lk_global_set(".str.on assign", LK_OBJ(self->str_onassign = lk_str_new_fromcstr(self, ".on_assign")));
    lk_global_set(".str.at", LK_OBJ(self->str_at = lk_str_new_fromcstr(self, "at")));
    lk_global_set(".str.slash", LK_OBJ(self->str_filesep = lk_str_new_fromcstr(self, "/")));

    /* attach all funcs to primitive types */
    lk_bool_libinit(self);
    lk_char_libinit(self);
    lk_charset_libinit(self);
    lk_map_libinit(self);
    lk_err_libinit(self);
    lk_file_libinit(self);
    lk_dir_libinit(self);
    lk_vec_libinit(self);
    lk_num_libinit(self);
    lk_scope_libinit(self);
    lk_func_libinit(self);
    lk_gc_libinit(self);
    lk_instr_libinit(self);
    lk_list_libinit(self);
    lk_obj_libinit(self);
    lk_parser_libinit(self);
    lk_pipe_libinit(self);
    lk_str_libinit(self);
    lk_vm_libinit(self);
    lk_seq_libinit(self);
    lk_dl_libinit(self);

    /* extra libs */
    lk_env_extinit(self);
    lk_rand_extinit(self);
    lk_socket_extinit(self);
    lk_global_set("Global", self->global);
    return self;
}
void lk_vm_free(lk_vm_t *self) {
    lk_gc_t *gc = self->gc;
    lk_objgroup_remove(LK_OBJ(gc));
    lk_gc_free_objgroup(gc->unused);
    lk_gc_free_objgroup(gc->pending);
    lk_gc_free_objgroup(gc->used);
    lk_obj_justfree(LK_OBJ(gc));
    mem_free(self);

    /*
    fprintf(stderr, "allocsize: %i\n", mem_allocsize());
    fprintf(stderr, "alloctotal: %i\n", mem_alloctotal());
    fprintf(stderr, "allocpeak: %i\n", mem_allocpeak());
    fprintf(stderr, "allocused: %i\n", mem_allocused());
    */
}

/* eval */
static lk_scope_t *eval(lk_vm_t *self, lk_str_t *code) {
    lk_parser_t *p = lk_parser_new(self);
    lk_instr_t *func = lk_parser_parse(p, code);
    lk_scope_t *fr = lk_scope_new(self);
    fr->first = fr->next = func;
    fr->returnto = NULL;
    lk_vm_doevalfunc(self);
    return fr;
}
lk_scope_t *lk_vm_evalfile(lk_vm_t *self, const char *file, const char *base) {
    lk_str_t *filename = lk_str_new_fromcstr(self, file);
    if(base != NULL && file[0] != '/') {
        lk_str_t *root = lk_str_new_fromcstr(self, base);
        int pos, i, pslen = DARRAY_COUNT(DARRAY(self->str_filesep));
        pos = darray_find_darray(DARRAY(root), DARRAY(self->str_filesep), 0);
        if(pos > 0) {
            lk_str_t *orig = filename;
            root = lk_str_new_fromdarray(self, DARRAY(root));
            pos += pslen;
            while((i = darray_find_darray(
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
        const char *cfilename = darray_str_tocstr(DARRAY(filename));
        rsrc.isstr = 0;
        rsrc.rsrc = filename;
        rsrc.prev = self->rsrc;
        self->rsrc = &rsrc;
        stream = fopen(cfilename, "r");
        if(stream != NULL) {
            darray_t *src = darray_str_alloc_fromfile(stream);
            fclose(stream);
            if(src != NULL) {
                fr = eval(self, lk_str_new_fromdarray(self, src));
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
lk_scope_t *lk_vm_evalstr(lk_vm_t *self, const char *code) {
    lk_scope_t *fr;
    struct lk_rsrcchain rsrc;
    rsrc.isstr = 1;
    rsrc.rsrc = lk_str_new_fromcstr(self, code);
    rsrc.prev = self->rsrc;
    self->rsrc = &rsrc;
    fr = eval(self, rsrc.rsrc);
    self->rsrc = self->rsrc->prev;
    return fr;
}
#define CALLFUNC(self, func, args) do { \
    (args)->argc = DARRAY_ISINIT(&(args)->stack) \
    ? DARRAY_COUNT(&(args)->stack) : 0; \
    if(LK_OBJ_ISCFUNC(LK_OBJ(func))) { \
        if(LK_CFUNC(func)->cc == LK_CFUNC_CC_CVOID) { \
            switch((args)->argc) { \
                case 0: LK_CFUNC(func)->cfunc.v0((args)->receiver); break; \
                case 1: LK_CFUNC(func)->cfunc.v1((args)->receiver, LK_OBJ(DARRAY_ATPTR(&args->stack, (0)))); break; \
                case 2: LK_CFUNC(func)->cfunc.v2((args)->receiver, LK_OBJ(DARRAY_ATPTR(&args->stack, (0))), LK_OBJ(DARRAY_ATPTR(&args->stack, (1)))); break; \
                case 3: LK_CFUNC(func)->cfunc.v3((args)->receiver, LK_OBJ(DARRAY_ATPTR(&args->stack, (0))), LK_OBJ(DARRAY_ATPTR(&args->stack, (1))), LK_OBJ(DARRAY_ATPTR(&args->stack, (2)))); break; \
                case 4: LK_CFUNC(func)->cfunc.v4((args)->receiver, LK_OBJ(DARRAY_ATPTR(&args->stack, (0))), LK_OBJ(DARRAY_ATPTR(&args->stack, (1))), LK_OBJ(DARRAY_ATPTR(&args->stack, (2))), LK_OBJ(DARRAY_ATPTR(&args->stack, 3))); break; \
                case 5: LK_CFUNC(func)->cfunc.v5((args)->receiver, LK_OBJ(DARRAY_ATPTR(&args->stack, (0))), LK_OBJ(DARRAY_ATPTR(&args->stack, (1))), LK_OBJ(DARRAY_ATPTR(&args->stack, (2))), LK_OBJ(DARRAY_ATPTR(&args->stack, 3)), LK_OBJ(DARRAY_ATPTR(&args->stack, 4))); break; \
                default: BUG("cc void not supported"); \
            } \
            lk_scope_stackpush((args)->returnto, (args)->receiver); \
        } else if(LK_CFUNC(func)->cc == LK_CFUNC_CC_CRETURN) { \
            switch((args)->argc) { \
                case 0: lk_scope_stackpush((args)->returnto, LK_CFUNC(func)->cfunc.r0((args)->receiver)); break; \
                case 1: lk_scope_stackpush((args)->returnto, LK_CFUNC(func)->cfunc.r1((args)->receiver, LK_OBJ(DARRAY_ATPTR(&args->stack, (0))))); break; \
                case 2: lk_scope_stackpush((args)->returnto, LK_CFUNC(func)->cfunc.r2((args)->receiver, LK_OBJ(DARRAY_ATPTR(&args->stack, (0))), LK_OBJ(DARRAY_ATPTR(&args->stack, (1))))); break; \
                case 3: lk_scope_stackpush((args)->returnto, LK_CFUNC(func)->cfunc.r3((args)->receiver, LK_OBJ(DARRAY_ATPTR(&args->stack, (0))), LK_OBJ(DARRAY_ATPTR(&args->stack, (1))), LK_OBJ(DARRAY_ATPTR(&args->stack, (2))))); break; \
                case 4: lk_scope_stackpush((args)->returnto, LK_CFUNC(func)->cfunc.r4((args)->receiver, LK_OBJ(DARRAY_ATPTR(&args->stack, (0))), LK_OBJ(DARRAY_ATPTR(&args->stack, (1))), LK_OBJ(DARRAY_ATPTR(&args->stack, (2))), LK_OBJ(DARRAY_ATPTR(&args->stack, 3)))); break; \
                case 5: lk_scope_stackpush((args)->returnto, LK_CFUNC(func)->cfunc.r5((args)->receiver, LK_OBJ(DARRAY_ATPTR(&args->stack, (0))), LK_OBJ(DARRAY_ATPTR(&args->stack, (1))), LK_OBJ(DARRAY_ATPTR(&args->stack, (2))), LK_OBJ(DARRAY_ATPTR(&args->stack, 3)), LK_OBJ(DARRAY_ATPTR(&args->stack, 4)))); break; \
                default: BUG("cc return not supported"); \
            } \
        } else { \
            LK_CFUNC(func)->cfunc.lk((args)->receiver, (args)); \
        } \
        vm->currscope = (self); \
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
    lk_scope_t *self = vm->currscope;
    lk_gc_t *gc = vm->gc;
    lk_instr_t *instr;
    lk_scope_t *args;
    /* freq used types */
    lk_obj_t *t_func = vm->t_func;
    /* used in slot resolution */
    lk_instr_t *msg;
    lk_str_t *msgn;
    qphash_t *slots;
    setitem_t *si;
    struct lk_slot *slot;
    darray_t *ancs;
    int anci, ancc;
    lk_obj_t *recv, *r, *slotv;
    lk_func_t *func;
    /* rescue err and run approp func */
    struct lk_rescue rescue;
    rescue.prev = vm->rescue;
    rescue.rsrc = vm->rsrc;
    vm->rescue = &rescue;
    if(setjmp(rescue.buf)) {
        recv = LK_OBJ(self->scope);
        args = lk_scope_new(vm);
        lk_scope_stackpush(args, LK_OBJ(vm->lasterr));
        for(; recv != NULL; recv = LK_OBJ(LK_SCOPE(recv)->returnto)) {
            if((slots = recv->o.slots) == NULL) continue;
            if((si = qphash_get(slots, vm->str_rescue)) == NULL) continue;
            slot = LK_SLOT(SETITEM_VALUEPTR(si));
            slotv = lk_obj_getvaluefromslot(recv, slot);
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
        lk_vm_raiseerr(vm, vm->lasterr);
    }
    nextinstr:
    /* gc mem? sweep triggered by mark if necessary */
    if(gc->newvalues > 5000) {
        lk_gc_mark(gc);
        gc->newvalues = 0;
    }
    /* like how cpu execs instrs by following a program sizeer */
    if((instr = self->next) == NULL) goto prevscope;
    vm->stat.totalinstrs ++;
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
        lk_scope_stackpush(self, lk_obj_clone(instr->v));
        goto nextinstr;
    /* funcs also need ref to env for closures to work */
    case LK_INSTRTYPE_FUNC: {
        lk_kfunc_t *clone = LK_KFUNC(lk_obj_clone(instr->v));
        lk_kfunc_updatesig(clone);
        lk_obj_addref(LK_OBJ(clone), LK_OBJ(self->scope));
        clone->scope = self->scope;
        lk_obj_addref(LK_OBJ(clone), LK_OBJ(self->receiver));
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
        vm->currscope = vm->currscope->caller;
        return;
    }
    switch(args->type) {
    /* take the scope and return last val */
    case LK_SCOPETYPE_RETURN:
        lk_scope_stackpush(self, lk_scope_stackpeek(args));
        vm->currscope = self;
        goto nextinstr;
    /* take the scope and convert to list */
    case LK_SCOPETYPE_LIST:
        lk_scope_stackpush(self, LK_OBJ(lk_scope_stacktolist(args)));
        vm->currscope = self;
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
        slotv = lk_obj_getvaluefromslot(recv, slot);
        /* slot contains func obj - call? */
        if(LK_OBJ_ISA(slotv, t_func) > 2
        && LK_SLOT_CHECKOPTION(slot, LK_SLOTOPTION_AUTOSEND)
        && (instr == NULL
        || instr->next == NULL
        || instr->next->type != LK_INSTRTYPE_APPLYMSG
        || darray_str_cmp_cstr(DARRAY(instr->next->v), "+=") != 0)) {
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
                } else if(DARRAY_ISINIT(&args->stack)
                       && DARRAY_COUNT(&args->stack) > 0) {
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
            ancc = DARRAY_COUNT(ancs);
            for(anci = 1; anci < ancc; anci ++) {
                r = DARRAY_ATPTR(ancs, anci);
                if((slots = r->o.slots) == NULL) continue;
                if((si = qphash_get(slots, msg->v)) == NULL) continue;
                goto found;
            }
        } else {
            r = r->o.parent;
            goto findslot;
        }
        /* forward: */
        if(DARRAY_EQ(DARRAY(msgn), DARRAY(vm->str_forward))) {
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
    lk_err_t *err = LK_ERROR(lk_obj_alloc(self->t_err));
    va_list ap;
    err->instr = self->currinstr;
    err->message = LK_STRING(lk_obj_alloc(self->t_str));
    va_start(ap, message);
    for(; *message != '\0'; message ++) {
        if(*message == '%') {
            message ++;
            switch(*message) {
            case 's':
                darray_concat(DARRAY(err->message), DARRAY(va_arg(ap, lk_str_t *)));
                break;
            }
        } else {
            darray_str_push(DARRAY(err->message), *message);
        }
    }
    va_end(ap);
    lk_vm_raiseerr(self, err);
}
void lk_vm_raiseerrno(lk_vm_t *self) {
    lk_err_t *err = LK_ERROR(lk_obj_alloc(self->t_err));
    err->message = lk_str_new_fromcstr(self, strerror(errno));
    lk_vm_raiseerr(self, err);
}
void lk_vm_raiseerr(lk_vm_t *self, lk_err_t *err) {
    if(self->rescue == NULL) lk_vm_abort(self, err);
    else {
        self->lasterr = err;
        longjmp(self->rescue->buf, 1);
    }
}
void lk_vm_exit(lk_vm_t *self) {
    lk_vm_free(self);
    exit(EXIT_SUCCESS);
}
void lk_vm_abort(lk_vm_t *self, lk_err_t *err) {
    if(err != NULL) {
        struct lk_slot *slot = lk_obj_getslotfromany(
        LK_OBJ(err), LK_OBJ(self->str_type));
        lk_str_t *type = LK_STRING(lk_obj_getvaluefromslot(LK_OBJ(err), slot));
        lk_instr_t *expr = err->instr;
        int i = 0;
        darray_print_tostream(DARRAY(type), stdout);
        fprintf(stdout, "\n* rsrc: ");
        darray_print_tostream(DARRAY(expr->rsrc), stdout);
        fprintf(stdout, "\n* line: %i", expr->line);
        while(expr->prev != NULL) { expr = expr->prev; i ++; }
        fprintf(stdout, "\n* instruction(%i): ", i);
        lk_instr_print(expr);
        fprintf(stdout, "\n* text: ");
        if(err->message != NULL) darray_print_tostream(DARRAY(err->message), stdout);
        printf("\n");
    } else {
        printf("Unknown err!\n");
    }
    lk_vm_free(self);
    exit(EXIT_FAILURE);
}
