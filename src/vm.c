#include "vm.h"
#include "bool.h"
#include "char.h"
#include "charset.h"
#include "dir.h"
#include "env.h"
#include "err.h"
#include "file.h"
#include "func.h"
#include "gc.h"
#include "instr.h"
#include "lib.h"
#include "list.h"
#include "map.h"
#include "num.h"
#include "obj.h"
#include "parser.h"
#include "rand.h"
#include "scope.h"
#include "seq.h"
#include "socket.h"
#include "str.h"
#include "vec.h"
#include <errno.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// ext map - types
void lk_vm_type_init(lk_vm_t *vm) {
    vm->t_vm = lk_obj_alloc_type(vm->t_obj, sizeof(lk_obj_t));
}

// ext map - funcs
static void exit_vm(lk_obj_t *self, lk_scope_t *local) {
    (void)local;
    lk_vm_exit(VM);
    DONE;
}

static void fork_vm(lk_obj_t *self, lk_scope_t *local) {
    pid_t child = fork();

    if (child == -1)
        lk_vm_raise_errno(VM);

    RETURN(lk_num_new(VM, (int)child));
}

static void fork_vm_f(lk_obj_t *self, lk_scope_t *local) {
    pid_t child = fork();

    if (child == -1)
        lk_vm_raise_errno(VM);

    if (child > 0)
        RETURN(lk_num_new(VM, (int)child));

    else {
        lk_lfunc_t *lf = LK_LFUNC(ARG(0));
        lk_scope_t *fr = lk_scope_new(VM);

        fr->first = fr->next = lf->first;
        fr->receiver = fr->self = self;
        fr->func = LK_OBJ(lf);
        fr->returnto = NULL;
        fr->parent = lf->scope;
        lk_vm_do_eval_func(VM);
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
    offset += (long)(l.tm_min - gm.tm_min) * 60;
    offset += (long)(l.tm_hour - gm.tm_hour) * 3600;

    if (l.tm_year < gm.tm_year || l.tm_mon < gm.tm_mon || l.tm_mday < gm.tm_mday) {
        offset -= 86400;
    }

    RETURN(lk_num_new(VM, offset));
}

static void system_vm(lk_obj_t *self, lk_scope_t *local) {
    pid_t child = fork();

    if (child == -1)
        lk_vm_raise_errno(VM);

    if (child > 0) {
        int status;

        waitpid(child, &status, 0);
        RETURN(lk_num_new(VM, WEXITSTATUS(status)));

    } else {
        int c = local->argc;
        char **args = (char **)mem_alloc(sizeof(char *) * (c + 1));

        for (int i = 0; i < c; i++) {
            args[i] = (char *)vec_str_tocstr(VEC(ARG(i)));
        }

        execvp(args[0], args);
        lk_vm_raise_errno(VM);
    }
}

static void system2_vm_str(lk_obj_t *self, lk_scope_t *local) {
    FILE *out = popen(vec_str_tocstr(VEC(ARG(0))), "r");

    if (out != NULL) {
        char ret[4096];
        char *line = fgets(ret, 4096, out);
        pclose(out);

        if (line != NULL)
            RETURN(lk_str_new_from_cstr(VM, line));
    }

    RETURN(NIL);
}

static void wait_vm(lk_obj_t *self, lk_scope_t *local) {
    int status;
    pid_t child = wait(&status);

    RETURN(lk_num_new(VM, (int)child));
}

void lk_vm_lib_init(lk_vm_t *vm) {
    lk_obj_t *tvm = vm->t_vm, *f = vm->t_func, *num = vm->t_num, *str = vm->t_str;

    lk_global_set("VirtualMachine", tvm);
    lk_obj_set_cfunc_lk(tvm, "exit", exit_vm, NULL);
    lk_obj_set_cfunc_lk(tvm, "fork", fork_vm, NULL);
    lk_obj_set_cfunc_lk(tvm, "fork", fork_vm_f, f, NULL);
    lk_obj_set_cfunc_lk(tvm, "sleep", sleep_vm_num, num, NULL);
    lk_obj_set_cfunc_lk(tvm, "seconds since epoch", seconds_since_epoch_vm, NULL);
    lk_obj_set_cfunc_lk(tvm, "seconds west of utc", seconds_west_of_utc_vm, NULL);
    lk_obj_set_cfunc_lk(tvm, "system", system_vm, (lk_obj_t *)-1);
    lk_obj_set_cfunc_lk(tvm, "system2", system2_vm_str, str, NULL);
    lk_obj_set_cfunc_lk(tvm, "wait", wait_vm, NULL);
}

// new
lk_vm_t *lk_vm_new(void) {
    lk_vm_t *self = mem_alloc(sizeof(lk_vm_t));

    // must be loaded before other primitive types
    lk_obj_type_init(self);
    lk_gc_type_init(self);
    lk_view_type_init(self);
    lk_vm_type_init(self);
    lk_seq_type_init(self);
    lk_map_type_init(self);
    lk_str_type_init(self);

    // init all other primitive types
    lk_bool_type_init(self);
    lk_char_type_init(self);
    lk_charset_type_init(self);
    lk_err_type_init(self);
    lk_cptr_type_init(self);
    lk_file_type_init(self);
    lk_dir_type_init(self);
    lk_vec_type_init(self);
    lk_num_type_init(self);
    lk_scope_type_init(self);
    lk_func_type_init(self);
    lk_instr_type_init(self);
    lk_list_type_init(self);
    lk_parser_type_init(self);
    lk_pipe_type_init(self);
    lk_dl_type_init(self);

    // init rest of the fields in vm
    self->global = LK_SCOPE(lk_obj_alloc(self->t_scope));
    self->currscope = self->global;
    lk_global_set(".str.class", LK_OBJ(self->str_type = lk_str_new_from_cstr(self, "_CLASS")));
    lk_global_set(".str.forward", LK_OBJ(self->str_forward = lk_str_new_from_cstr(self, "_forward")));
    lk_global_set(".str.rescue", LK_OBJ(self->str_rescue = lk_str_new_from_cstr(self, "_rescue")));
    lk_global_set(".str.on assign", LK_OBJ(self->str_onassign = lk_str_new_from_cstr(self, "_on_assign")));
    lk_global_set(".str.at", LK_OBJ(self->str_at = lk_str_new_from_cstr(self, "at")));
    lk_global_set(".str.slash", LK_OBJ(self->str_filesep = lk_str_new_from_cstr(self, "/")));
    lk_global_set(".str.lktype", LK_OBJ(self->str_lktype = lk_str_new_from_cstr(self, "_type")));

    // attach all funcs to primitive types
    lk_bool_lib_init(self);
    lk_char_lib_init(self);
    lk_charset_lib_init(self);
    lk_map_lib_init(self);
    lk_err_lib_init(self);
    lk_cptr_lib_init(self);
    lk_file_lib_init(self);
    lk_dir_lib_init(self);
    lk_vec_lib_init(self);
    lk_num_lib_init(self);
    lk_scope_lib_init(self);
    lk_func_lib_init(self);
    lk_gc_lib_init(self);
    lk_instr_lib_init(self);
    lk_list_lib_init(self);
    lk_obj_lib_init(self);
    lk_parser_lib_init(self);
    lk_pipe_lib_init(self);
    lk_str_lib_init(self);
    lk_vm_lib_init(self);
    lk_seq_lib_init(self);
    lk_dl_lib_init(self);

    // extra libs
    lk_env_ext_init(self);
    lk_rand_ext_init(self);
    lk_socket_ext_init(self);
    lk_global_set("Global", LK_OBJ(self->global));
    return self;
}

void lk_vm_free(lk_vm_t *self) {
    lk_gc_t *gc = self->gc;

    lk_objgroup_remove(LK_OBJ(gc));
    lk_gc_free_objgroup(gc->unused);
    lk_gc_free_objgroup(gc->pending);
    lk_gc_free_objgroup(gc->used);
    lk_obj_just_free(LK_OBJ(gc));
    mem_free(self);

    /*
    fprintf(stderr, "allocsize: %i\n", mem_alloc_count());
    fprintf(stderr, "alloctotal: %i\n", mem_alloc_total());
    fprintf(stderr, "allocpeak: %i\n", mem_alloc_peak());
    fprintf(stderr, "allocused: %i\n", mem_alloc_used());
    */
}

// eval
static lk_scope_t *eval(lk_vm_t *self, lk_str_t *code) {
    lk_parser_t *p = lk_parser_new(self);
    lk_instr_t *func = lk_parser_parse(p, code);
    lk_scope_t *fr = lk_scope_new(self);

    fr->first = fr->next = func;
    fr->returnto = NULL;
    lk_vm_do_eval_func(self);
    return fr;
}

lk_scope_t *lk_vm_eval_file(lk_vm_t *self, const char *file, const char *base) {
    lk_str_t *filename = lk_str_new_from_cstr(self, file);

    if (base != NULL && file[0] != '/') {
        lk_str_t *root = lk_str_new_from_cstr(self, base);
        int pslen = VEC_COUNT(VEC(self->str_filesep));
        int pos = vec_find_vec(VEC(root), VEC(self->str_filesep), 0);

        if (pos > 0) {
            lk_str_t *orig = filename;

            root = lk_str_new_from_darray(self, VEC(root));
            pos += pslen;

            for (int i; (i = vec_find_vec(VEC(root), VEC(self->str_filesep), pos)) > 0;)
                pos = i + pslen;

            vec_slice(VEC(root), 0, pos);
            vec_resize_item(VEC(root), VEC(orig));
            vec_concat(VEC(root), VEC(orig));
            filename = root;
        }
    }

    {
        lk_scope_t *fr;
        struct lk_rsrcchain rsrc;
        FILE *stream;
        const char *cfilename = vec_str_tocstr(VEC(filename));

        rsrc.isstr = 0;
        rsrc.rsrc = filename;
        rsrc.prev = self->rsrc;
        self->rsrc = &rsrc;
        stream = fopen(cfilename, "r");

        if (stream != NULL) {
            vec_t *src = vec_str_alloc_from_file(stream);

            fclose(stream);

            if (src != NULL) {
                fr = eval(self, lk_str_new_from_darray(self, src));
                vec_free(src);
                self->rsrc = self->rsrc->prev;
                return fr;

            } else {
                self->rsrc = self->rsrc->prev;
                lk_vm_raise_cstr(self, "Cannot read from file named %s", filename);
            }

        } else {
            self->rsrc = self->rsrc->prev;
            lk_vm_raise_cstr(self, "Cannot open file named %s", filename);
        }
    }
}

lk_scope_t *lk_vm_eval_str(lk_vm_t *self, const char *code) {
    lk_scope_t *fr;
    struct lk_rsrcchain rsrc;

    rsrc.isstr = 1;
    rsrc.rsrc = lk_str_new_from_cstr(self, code);
    rsrc.prev = self->rsrc;
    self->rsrc = &rsrc;
    fr = eval(self, rsrc.rsrc);
    self->rsrc = self->rsrc->prev;
    return fr;
}

/* dispatch a C function call with up to 5 arguments.
   argc must be set on args before calling. */
static void call_cfunc(lk_vm_t *vm, lk_scope_t *self, lk_cfunc_t *cf, lk_scope_t *args) {
    lk_obj_t *recv = args->receiver;
    int argc = args->argc;
#define A(i) LK_OBJ(VEC_ATPTR(&args->stack, (i)))

    if (cf->cc == LK_CFUNC_CC_CVOID) {
        switch (argc) {
        case 0:
            cf->cfunc.v0(recv);
            break;
        case 1:
            cf->cfunc.v1(recv, A(0));
            break;
        case 2:
            cf->cfunc.v2(recv, A(0), A(1));
            break;
        case 3:
            cf->cfunc.v3(recv, A(0), A(1), A(2));
            break;
        case 4:
            cf->cfunc.v4(recv, A(0), A(1), A(2), A(3));
            break;
        case 5:
            cf->cfunc.v5(recv, A(0), A(1), A(2), A(3), A(4));
            break;
        default:
            BUG("cc void not supported");
        }

        lk_scope_stack_push(args->returnto, recv);

    } else if (cf->cc == LK_CFUNC_CC_CRETURN) {
        lk_obj_t *result;

        switch (argc) {
        case 0:
            result = cf->cfunc.r0(recv);
            break;
        case 1:
            result = cf->cfunc.r1(recv, A(0));
            break;
        case 2:
            result = cf->cfunc.r2(recv, A(0), A(1));
            break;
        case 3:
            result = cf->cfunc.r3(recv, A(0), A(1), A(2));
            break;
        case 4:
            result = cf->cfunc.r4(recv, A(0), A(1), A(2), A(3));
            break;
        case 5:
            result = cf->cfunc.r5(recv, A(0), A(1), A(2), A(3), A(4));
            break;
        default:
            BUG("cc return not supported");
        }

        lk_scope_stack_push(args->returnto, result);

    } else {
        cf->cfunc.lk(recv, args);
    }

#undef A
    vm->currscope = self;
}

/* CALLFUNC must stay a macro: the kfunc branch mutates `self` (a local in the
   eval loop) and both branches end with goto nextinstr. */
#define CALLFUNC(self, func, args) \
    do { \
        (args)->argc = VEC_ISINIT(&(args)->stack) ? VEC_COUNT(&(args)->stack) : 0; \
        if (LK_OBJ_ISCFUNC(LK_OBJ(func))) { \
            call_cfunc(vm, self, LK_CFUNC(func), args); \
        } else { \
            (args)->parent = LK_LFUNC(func)->scope; \
            (args)->self = LK_OBJ_ISSCOPE((args)->receiver) ? LK_LFUNC(func)->scope->self : (args)->receiver; \
            (args)->first = (args)->next = LK_LFUNC(func)->first; \
            (self) = (args); \
        } \
        goto nextinstr; \
    } while (0)

void lk_vm_do_eval_func(lk_vm_t *vm) {
    lk_scope_t *self = vm->currscope;
    lk_gc_t *gc = vm->gc;
    lk_instr_t *instr;
    lk_scope_t *args;

    // used in slot resolution
    lk_instr_t *msg;
    lk_str_t *msgn;
    ht_t *slots;
    ht_item_t *si;
    struct lk_slot *slot;
    vec_t *ancs;
    lk_obj_t *recv, *r, *slotv;
    lk_func_t *func;
    int dispatch_phase;

    // rescue err and run approp func
    struct lk_rescue rescue;

    rescue.prev = vm->rescue;
    rescue.rsrc = vm->rsrc;
    vm->rescue = &rescue;

    if (setjmp(rescue.buf)) {
        recv = vm->currscope != NULL ? LK_OBJ(vm->currscope) : (self != NULL ? LK_OBJ(self->scope) : NULL);
        args = lk_scope_new(vm);
        lk_scope_stack_push(args, LK_OBJ(vm->lasterr));

        for (; recv != NULL; recv = LK_OBJ(LK_SCOPE(recv)->returnto)) {
            if ((slots = recv->o.slots) == NULL)
                continue;
            if ((si = ht_get(slots, vm->str_rescue)) == NULL)
                continue;
            slot = LK_SLOT(HT_ITEM_VALUEPTR(si));
            slotv = lk_obj_get_value_from_slot(recv, slot);
            if (!LK_OBJ_ISFUNC(slot->check) || !LK_OBJ_ISCALLABLE(slotv))
                continue;
            func = lk_func_match(LK_FUNC(slotv), args, args->self);
            if (func == NULL)
                continue;
            args->receiver = recv;
            args->returnto = LK_SCOPE(recv)->returnto;
            args->func = slotv; // LK_OBJ(func);
            CALLFUNC(self, func, args);
        }

        vm->rescue = vm->rescue->prev;
        vm->rsrc = vm->rescue != NULL ? vm->rescue->rsrc : NULL;
        lk_vm_raise_err(vm, vm->lasterr);
    }

nextinstr:

    // gc mem? sweep triggered by mark if necessary
    if (gc->newvalues > 5000) {
        lk_gc_mark(gc);
        gc->newvalues = 0;
    }

    // like how cpu execs instrs by following a program sizeer
    if ((instr = self->next) == NULL)
        goto prevscope;
    vm->stat.totalinstrs++;
    vm->currinstr = self->current = instr;
    self->next = instr->next;

    switch (instr->type) {
    // skip comments
    // msg represents a possiblity, a function to be exec'd
    case LK_INSTRTYPE_SELFMSG:
        lk_scope_stack_push(self, self->self != NULL ? self->self : NIL);
        goto sendmsg;
    case LK_INSTRTYPE_SCOPEMSG:
        lk_scope_stack_push(self, LK_OBJ(self->scope));
        goto sendmsg;
    case LK_INSTRTYPE_SLOTMSG:
    case LK_INSTRTYPE_APPLYMSG:
    sendmsg:
        lk_scope_stack_push(self, LK_OBJ(instr));
        if (instr->opts & LK_INSTROHASMSGARGS)
            goto nextinstr;
        args = NULL;
        goto apply;
    case LK_INSTRTYPE_APPLY:
    case LK_INSTRTYPE_LIST:
        args = lk_scope_new(vm);
        args->first = args->next = LK_INSTR(instr->v);
        args->type = instr->type == LK_INSTRTYPE_APPLY ? LK_SCOPETYPE_APPLY : LK_SCOPETYPE_LIST;
        args->scope = self->scope;
        args->receiver = self->receiver;
        self = args;
        goto nextinstr;

    // "literals" are actually generators
    case LK_INSTRTYPE_NUMBER:
    case LK_INSTRTYPE_STRING:
    case LK_INSTRTYPE_CHAR:
        lk_scope_stack_push(self, lk_obj_clone(instr->v));
        goto nextinstr;

    // funcs also need ref to env for closures to work
    case LK_INSTRTYPE_FUNC: {
        lk_lfunc_t *clone = LK_LFUNC(lk_obj_clone(instr->v));

        lk_lfunc_update_sig(clone);
        lk_obj_add_ref(LK_OBJ(clone), LK_OBJ(self->scope));
        clone->scope = self->scope;
        lk_obj_add_ref(LK_OBJ(clone), LK_OBJ(self->receiver));
        lk_scope_stack_push(self, LK_OBJ(clone));
        goto nextinstr;
    }

    // read more instr from the parser
    case LK_INSTRTYPE_MORE:
        self->next = lk_parser_getmore(LK_PARSER(instr->v));
        goto nextinstr;

    // should never happen
    default:
        BUG("Invalid instruction type");
    }

// return from func
prevscope:
    args = self;
    self = self->returnto;

    if (self == NULL) {
        vm->rescue = vm->rescue->prev;
        vm->currscope = vm->currscope->caller;
        return;
    }

    switch (args->type) {
    // take the scope and return last val
    case LK_SCOPETYPE_RETURN:
        lk_scope_stack_push(self, lk_scope_stack_peek(args));
        vm->currscope = self;
        goto nextinstr;

    // take the scope and convert to list
    case LK_SCOPETYPE_LIST:
        lk_scope_stack_push(self, LK_OBJ(lk_scope_stack_to_list(args)));
        vm->currscope = self;
        goto nextinstr;

    // take the scope and use as args for msg
    case LK_SCOPETYPE_APPLY:
    apply:
        /*
        msg = LK_INSTR(lk_scope_stack_pop(self));
        msgn = LK_STRING(msg->v);
        recv = r = lk_scope_stack_pop(self);
        */
        recv = r = lk_scope_stack_pop(self);

        if (LK_OBJ_ISINSTR(recv)) {
            msg = LK_INSTR(recv);
            msgn = LK_STRING(msg->v);
            recv = r = lk_scope_stack_pop(self);

        } else {
            msg = NULL;
            msgn = vm->str_at;
        }

        if (msg != NULL && msg->type == LK_INSTRTYPE_SLOTMSG) {
            struct lk_slot *slot = lk_obj_getslot(recv, LK_OBJ(msgn));
            lk_scope_stack_push(self, slot != NULL ? lk_obj_get_value_from_slot(recv, slot) : NIL);
            goto nextinstr;
        }

        // Phase 1: instance slots (recv directly)
        dispatch_phase = 0;

        if ((slots = recv->o.slots) != NULL && (si = ht_get(slots, msgn)) != NULL) {
            r = recv;
            goto found;
        }

    view_dispatch:
        // Phase 2: view type slots (flat — one lookup, no chain walk)
        dispatch_phase = 1;

        if (recv->o.view->type != NULL) {
            r = recv->o.view->type;

            if ((slots = r->o.slots) != NULL && (si = ht_get(slots, msgn)) != NULL)
                goto found;
        }

    type_chain:
        // Phase 3: _type chain or scope parent chain
        dispatch_phase = 2;
        ancs = NULL;

        if (LK_OBJ_ISSCOPE(recv) && LK_SCOPE(recv)->parent != NULL) {
            r = LK_OBJ(LK_SCOPE(recv)->parent);
            goto findslot;
        }

        r = NULL;

        if (recv->o.slots != NULL && (si = ht_get(recv->o.slots, vm->str_lktype)) != NULL) {
            r = lk_obj_get_value_from_slot(recv, LK_SLOT(HT_ITEM_VALUEPTR(si)));

            if (r == vm->t_nil)
                r = NULL;
        }

        if (r == NULL) {
            if (LK_OBJ_HASPARENTS(recv)) {
                r = recv;
                goto parent;
            }

            r = recv->o.view->parent;
        }

        if (r != NULL)
            goto findslot;

        goto forward;

    findslot:
        if ((slots = r->o.slots) == NULL)
            goto parent;
        if ((si = ht_get(slots, msgn)) == NULL)
            goto parent;
    found:
        slot = LK_SLOT(HT_ITEM_VALUEPTR(si));
        slotv = lk_obj_get_value_from_slot(recv, slot);

        // slot contains func obj - call?
        if (LK_OBJ_ISCALLABLE(slotv) && LK_SLOT_CHECKOPTION(slot, LK_SLOTOPTION_AUTOSEND) &&
            (instr == NULL || instr->next == NULL || instr->next->type != LK_INSTRTYPE_APPLYMSG ||
             vec_str_cmp_cstr(VEC(instr->next->v), "+=") != 0)) {
        callfunc:
            if (args == NULL)
                args = lk_scope_new(vm);
            func = lk_func_match(LK_FUNC(slotv), args, recv);
            if (func == NULL)
                goto parent;
            args->type = LK_SCOPETYPE_RETURN;
            args->scope = args;
            args->receiver = recv;
            args->func = slotv; // LK_OBJ(func);
            CALLFUNC(self, func, args);

        } else {
            // called like a func?
            if (args != NULL) {
                // slot contains func
                if (LK_OBJ_ISCALLABLE(slotv)) {
                    goto callfunc;

                    // call at/apply if there are args
                } else if (VEC_ISINIT(&args->stack) && VEC_COUNT(&args->stack) > 0) {
                    msgn = vm->str_at;
                    recv = r = slotv;
                    dispatch_phase = 0;

                    if ((slots = recv->o.slots) != NULL && (si = ht_get(slots, msgn)) != NULL)
                        goto found;
                    goto view_dispatch;
                }
            }

            self->lastslot = slot;
            lk_scope_stack_push(self, slotv);
            goto nextinstr;
        }
    parent:
        if (dispatch_phase < 2) {
            if (dispatch_phase == 0)
                goto view_dispatch;
            goto type_chain;
        }

        // if r is a non-scope instance (not a type), navigate to its type before using ancestors
        if (!LK_OBJ_ISSCOPE(r) && r->o.view->type != NULL && r->o.view->type != r) {
            r = r->o.view->type;
            goto findslot;
        }

        if ((ancs = r->o.view->ancestors) != NULL) {
            int ancc = VEC_COUNT(ancs);

            for (int anci = 1; anci < ancc; anci++) {
                r = VEC_ATPTR(ancs, anci);
                if ((slots = r->o.slots) == NULL)
                    continue;
                if ((si = ht_get(slots, msgn)) == NULL)
                    continue;
                goto found;
            }

        } else {
            r = (LK_OBJ_ISSCOPE(r) && LK_SCOPE(r)->parent != NULL) ? LK_OBJ(LK_SCOPE(r)->parent) : r->o.view->parent;

            if (r != NULL)
                goto findslot;
        }

    forward:
        if (VEC_EQ(VEC(msgn), VEC(vm->str_forward))) {
            lk_vm_raise_cstr(vm, "Cannot find slot named %s", msgn);

        } else {
            msgn = vm->str_forward;
            dispatch_phase = 0;

            if ((slots = recv->o.slots) != NULL && (si = ht_get(slots, msgn)) != NULL) {
                r = recv;
                goto found;
            }
            goto view_dispatch;
        }

    // should never happen
    default:
        BUG("Invalid scope type");
    }
}

void lk_vm_raise_cstr(lk_vm_t *self, const char *message, ...) {
    lk_obj_t *err = lk_obj_alloc(self->t_err);
    lk_str_t *msg = LK_STRING(lk_obj_alloc(self->t_str));
    va_list ap;

    if (self->currinstr != NULL)
        lk_obj_set_slot_by_cstr(err, "instr", NULL, LK_OBJ(self->currinstr));

    va_start(ap, message);

    for (; *message != '\0'; message++) {
        if (*message == '%') {
            message++;
            switch (*message) {
            case 's':
                vec_concat(VEC(msg), VEC(va_arg(ap, lk_str_t *)));
                break;
            }
        } else {
            vec_str_push(VEC(msg), *message);
        }
    }

    va_end(ap);
    lk_obj_set_slot_by_cstr(err, "message", NULL, LK_OBJ(msg));
    lk_vm_raise_err(self, err);
}

void lk_vm_raise_errno(lk_vm_t *self) {
    lk_obj_t *err = lk_obj_alloc(self->t_err);

    lk_obj_set_slot_by_cstr(err, "message", NULL, LK_OBJ(lk_str_new_from_cstr(self, strerror(errno))));
    lk_vm_raise_err(self, err);
}

void lk_vm_raise_err(lk_vm_t *self, lk_obj_t *err) {
    if (self->rescue == NULL)
        lk_vm_abort(self, err);
    else {
        self->lasterr = err;
        longjmp(self->rescue->buf, 1);
    }
}

void lk_vm_exit(lk_vm_t *self) {
    lk_vm_free(self);
    exit(EXIT_SUCCESS);
}

void lk_vm_abort(lk_vm_t *self, lk_obj_t *err) {
    if (err != NULL) {
        struct lk_slot *slot = lk_obj_get_slot_from_any(err, LK_OBJ(self->str_type));
        lk_str_t *type = LK_STRING(lk_obj_get_value_from_slot(err, slot));
        lk_obj_t *instr_obj = lk_obj_get_value_by_cstr(err, "instr");
        lk_instr_t *expr = (instr_obj != NULL && instr_obj != self->t_nil) ? LK_INSTR(instr_obj) : NULL;
        lk_obj_t *msg_obj = lk_obj_get_value_by_cstr(err, "message");
        int i = 0;

        vec_print_tostream(VEC(type), stdout);

        if (expr == NULL) {
            fprintf(stdout, "\n* rsrc: (no instr)");

        } else {
            fprintf(stdout, "\n* rsrc: ");
            if (expr->rsrc != NULL)
                vec_print_tostream(VEC(expr->rsrc), stdout);
            else
                fprintf(stdout, "(null)");
            fprintf(stdout, "\n* line: %i", expr->line);

            while (expr->prev != NULL) {
                expr = expr->prev;
                i++;
            }

            fprintf(stdout, "\n* instruction(%i): ", i);
            lk_instr_print(expr);
        }

        fprintf(stdout, "\n* text: ");
        if (msg_obj != NULL && msg_obj != self->t_nil)
            vec_print_tostream(VEC(LK_STRING(msg_obj)), stdout);
        printf("\n");

    } else {
        printf("Unknown err!\n");
    }

    lk_vm_free(self);
    exit(EXIT_FAILURE);
}
