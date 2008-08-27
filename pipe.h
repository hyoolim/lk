#ifndef LK_PIPE_H
#define LK_PIPE_H
#include "types.h"

/* type */
struct lk_pipe {
    struct lk_common  o;
    lk_str_t         *cmd;
    FILE             *fd;
};

/* init */
void lk_pipe_typeinit(lk_vm_t *vm);
void lk_pipe_libinit(lk_vm_t *vm);

/* update */
void lk_pipe_close(lk_pipe_t *self);
void lk_pipe_flush(lk_pipe_t *self);
void lk_pipe_init_str(lk_pipe_t *self, lk_str_t *cmd);
void lk_pipe_open(lk_pipe_t *self, lk_str_t *mode);
void lk_pipe_write_str(lk_pipe_t *self, lk_str_t *text);

/* info */
lk_str_t *lk_pipe_read_num(lk_pipe_t *self, lk_num_t *length);
lk_str_t *lk_pipe_readall(lk_pipe_t *self);
lk_str_t *lk_pipe_readuntil_char(lk_pipe_t *self, lk_char_t *until);
lk_str_t *lk_pipe_readuntil_charset(lk_pipe_t *self, lk_charset_t *until);
#endif
