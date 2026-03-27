#include "lib.h"
#include <errno.h>
#include <unistd.h>

// type
static void close_pipe_fd(void *ptr) {
    pclose((FILE *)ptr);
}

void lk_pipe_type_init(lk_vm_t *vm) {
    vm->t_pipe = lk_obj_alloc_type(vm->t_obj, sizeof(lk_obj_t));
}

// update
void lk_pipe_close(lk_obj_t *self) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr != NULL && cptr->ptr != NULL) {
        if (pclose((FILE *)cptr->ptr) != 0)
            lk_vm_raise_errno(VM);

        cptr->ptr = NULL;
    }
}

void lk_pipe_flush(lk_obj_t *self) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr != NULL && cptr->ptr != NULL && fflush((FILE *)cptr->ptr) != 0)
        lk_vm_raise_errno(VM);
}

void lk_pipe_init(lk_obj_t *self, lk_str_t *cmd) {
    lk_obj_set_slot_by_cstr(self, "cmd", NULL, LK_OBJ(cmd));
}

void lk_pipe_open(lk_obj_t *self, lk_str_t *mode) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));
    lk_str_t *cmd = LK_STRING(lk_obj_get_value_by_cstr(self, "cmd"));
    FILE *fd;

    if (cptr != NULL && cptr->ptr != NULL)
        BUG("ReadableFile->st.pipe should be NULL");

    fd = popen(CSTRING(cmd), CSTRING(mode));

    if (fd == NULL)
        lk_vm_raise_errno(VM);

    lk_obj_set_slot_by_cstr(self, "fd", NULL, LK_OBJ(lk_cptr_new(VM, fd, close_pipe_fd)));
}

void lk_pipe_write_str(lk_obj_t *self, lk_str_t *text) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr == NULL || cptr->ptr == NULL)
        BUG("WritableFile->st.pipe should NEVER be NULL");

    vec_print_tostream(VEC(text), (FILE *)cptr->ptr);
}

// info
lk_str_t *lk_pipe_read_num(lk_obj_t *self, lk_num_t *length) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr == NULL || cptr->ptr == NULL) {
        BUG("ReadableFile->st.pipe should NEVER be NULL");

    } else {
        vec_t *c = vec_str_alloc_from_file_with_length((FILE *)cptr->ptr, CNUMBER(length));
        return c != NULL ? lk_str_new_from_darray(VM, c) : LK_STRING(NIL);
    }
}

lk_str_t *lk_pipe_read_all(lk_obj_t *self) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr == NULL || cptr->ptr == NULL) {
        BUG("ReadableFile->st.pipe should NEVER be NULL");

    } else {
        vec_t *c = vec_str_alloc_from_file((FILE *)cptr->ptr);
        return c != NULL ? lk_str_new_from_darray(VM, c) : LK_STRING(NIL);
    }
}

lk_str_t *lk_pipe_read_until_char(lk_obj_t *self, lk_char_t *until) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr == NULL || cptr->ptr == NULL) {
        BUG("ReadableFile->st.pipe should NEVER be NULL");

    } else {
        vec_t *c = vec_str_alloc_from_file_until_char((FILE *)cptr->ptr, CHAR(until));
        return c != NULL ? lk_str_new_from_darray(VM, c) : LK_STRING(NIL);
    }
}

lk_str_t *lk_pipe_read_until_charset(lk_obj_t *self, lk_charset_t *until) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "fd"));

    if (cptr == NULL || cptr->ptr == NULL) {
        BUG("ReadableFile->st.pipe should NEVER be NULL");

    } else {
        vec_t *c = vec_str_alloc_from_file_until_charset((FILE *)cptr->ptr, CHARSET(until));
        return c != NULL ? lk_str_new_from_darray(VM, c) : LK_STRING(NIL);
    }
}

// bind all c funcs to lk equiv
void lk_pipe_lib_init(lk_vm_t *vm) {
    lk_obj_t *pipe = vm->t_pipe, *str = vm->t_str, *num = vm->t_num, *ch = vm->t_char, *charset = vm->t_charset;

    lk_global_set("Pipe", pipe);

    // update
    lk_obj_set_cfunc_cvoid(pipe, "close!", lk_pipe_close, NULL);
    lk_obj_set_cfunc_cvoid(pipe, "flush!", lk_pipe_flush, NULL);
    lk_obj_set_cfunc_cvoid(pipe, "init!", lk_pipe_init, str, NULL);
    lk_obj_set_cfunc_cvoid(pipe, "open", lk_pipe_open, str, NULL);
    lk_obj_set_cfunc_cvoid(pipe, "write", lk_pipe_write_str, str, NULL);

    // info
    lk_obj_set_cfunc_creturn(pipe, "read", lk_pipe_read_num, num, NULL);
    lk_obj_set_cfunc_creturn(pipe, "readAll", lk_pipe_read_all, NULL);
    lk_obj_set_cfunc_creturn(pipe, "readUntil", lk_pipe_read_until_char, ch, NULL);
    lk_obj_set_cfunc_creturn(pipe, "readUntil", lk_pipe_read_until_charset, charset, NULL);
}
