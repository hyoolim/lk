#ifndef LK_PIPE_H
#define LK_PIPE_H
#include "types.h"

// init
void lk_pipe_type_init(lk_vm_t *vm);
void lk_pipe_lib_init(lk_vm_t *vm);

// update
void lk_pipe_close(lk_obj_t *self);
void lk_pipe_flush(lk_obj_t *self);
void lk_pipe_init(lk_obj_t *self, lk_str_t *cmd);
void lk_pipe_open(lk_obj_t *self, lk_str_t *mode);
void lk_pipe_write_str(lk_obj_t *self, lk_str_t *text);

// info
lk_str_t *lk_pipe_read_num(lk_obj_t *self, lk_num_t *length);
lk_str_t *lk_pipe_read_all(lk_obj_t *self);
lk_str_t *lk_pipe_read_until_char(lk_obj_t *self, lk_char_t *until);
lk_str_t *lk_pipe_read_until_charset(lk_obj_t *self, lk_charset_t *until);
#endif
