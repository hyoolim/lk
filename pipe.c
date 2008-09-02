#include "lib.h"
#include <errno.h>
#include <unistd.h>

/* type */
static LK_OBJ_DEFMARKFUNC(mark_pipe) {
    mark(LK_OBJ(LK_PIPE(self)->cmd));
}
static void free_pipe(lk_obj_t *self) {
    lk_pipe_close(LK_PIPE(self));
}
void lk_pipe_typeinit(lk_vm_t *vm) {
    vm->t_pipe = lk_obj_alloc_withsize(vm->t_obj, sizeof(lk_pipe_t));
    lk_obj_setmarkfunc(vm->t_pipe, mark_pipe);
    lk_obj_setfreefunc(vm->t_pipe, free_pipe);
}

/* update */
void lk_pipe_close(lk_pipe_t *self) {
    if(self->fd != NULL) {
        if(pclose(self->fd) != 0) {
            lk_vm_raiseerrno(VM);
        }
        self->fd = NULL;
    }
}
void lk_pipe_flush(lk_pipe_t *self) {
    if(self->fd != NULL && fflush(self->fd) != 0) {
        lk_vm_raiseerrno(VM);
    }
}
void lk_pipe_init_str(lk_pipe_t *self, lk_str_t *path) {
    self->cmd = path;
}
void lk_pipe_open(lk_pipe_t *self, lk_str_t *mode) {
    if(self->fd != NULL) {
        BUG("ReadableFile->st.pipe should be NULL");
    }
    self->fd = popen(CSTRING(self->cmd), CSTRING(mode));
    if(self->fd == NULL) {
        lk_vm_raiseerrno(VM);
    }
}
void lk_pipe_write_str(lk_pipe_t *self, lk_str_t *text) {
    if(self->fd == NULL) {
        BUG("WritableFile->st.pipe should NEVER be NULL");
    }
    darray_print_tostream(DARRAY(text), self->fd);
}

/* info */
lk_str_t *lk_pipe_read_num(lk_pipe_t *self, lk_num_t *length) {
    if(self->fd == NULL) {
        BUG("ReadableFile->st.pipe should NEVER be NULL");
    } else {
        darray_t *c = darray_str_alloc_fromfile_withsize(self->fd, CNUMBER(length));
        return c != NULL ? lk_str_new_fromdarray(VM, c) : LK_STRING(NIL);
    }
}
lk_str_t *lk_pipe_readall(lk_pipe_t *self) {
    if(self->fd == NULL) {
        BUG("ReadableFile->st.pipe should NEVER be NULL");
    } else {
        darray_t *c = darray_str_alloc_fromfile(self->fd);
        return c != NULL ? lk_str_new_fromdarray(VM, c) : LK_STRING(NIL);
    }
}
lk_str_t *lk_pipe_readuntil_char(lk_pipe_t *self, lk_char_t *until) {
    if(self->fd == NULL) {
        BUG("ReadableFile->st.pipe should NEVER be NULL");
    } else {
        darray_t *c = darray_str_alloc_fromfile_untilchar(self->fd, CHAR(until));
        return c != NULL ? lk_str_new_fromdarray(VM, c) : LK_STRING(NIL);
    }
}
lk_str_t *lk_pipe_readuntil_charset(lk_pipe_t *self, lk_charset_t *until) {
    if(self->fd == NULL) {
        BUG("ReadableFile->st.pipe should NEVER be NULL");
    } else {
        darray_t *c = darray_str_alloc_fromfile_untilcharset(self->fd, CHARSET(until));
        return c != NULL ? lk_str_new_fromdarray(VM, c) : LK_STRING(NIL);
    }
}

/* bind all c funcs to lk equiv */
void lk_pipe_libinit(lk_vm_t *vm) {
    lk_obj_t *pipe = vm->t_pipe, *str = vm->t_str, *num = vm->t_num, *ch = vm->t_char, *charset = vm->t_charset;
    lk_global_set("Pipe", pipe);

    /* props */
    lk_obj_set_cfield(pipe, "command", str, offsetof(lk_pipe_t, cmd));

    /* update */
    lk_obj_set_cfunc_cvoid(pipe, "close!", lk_pipe_close, NULL);
    lk_obj_set_cfunc_cvoid(pipe, "flush!", lk_pipe_flush, NULL);
    lk_obj_set_cfunc_cvoid(pipe, "init!", lk_pipe_init_str, str, NULL);
    lk_obj_set_cfunc_cvoid(pipe, "open", lk_pipe_open, str, NULL);
    lk_obj_set_cfunc_cvoid(pipe, "write", lk_pipe_write_str, str, NULL);

    /* info */
    lk_obj_set_cfunc_creturn(pipe, "read", lk_pipe_read_num, num, NULL);
    lk_obj_set_cfunc_creturn(pipe, "readAll", lk_pipe_readall, NULL);
    lk_obj_set_cfunc_creturn(pipe, "readUntil", lk_pipe_readuntil_char, ch, NULL);
    lk_obj_set_cfunc_creturn(pipe, "readUntil", lk_pipe_readuntil_charset, charset, NULL);
}
