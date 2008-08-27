#ifndef LK_FILE_H
#define LK_FILE_H
#include "types.h"

/* type */
struct lk_file {
    struct lk_common  o;
    lk_str_t         *path;
    FILE             *fd;
};

/* init */
void lk_file_typeinit(lk_vm_t *vm);
void lk_file_libinit(lk_vm_t *vm);

/* update */
void lk_file_close(lk_file_t *self);
void lk_file_delete(lk_file_t *self);
void lk_file_flush(lk_file_t *self);
void lk_file_init_str(lk_file_t *self, lk_str_t *path);
void lk_file_move_str(lk_file_t *self, lk_str_t *dest);
void lk_file_open(lk_file_t *self, lk_str_t *mode);
void lk_file_write_str(lk_file_t *self, lk_str_t *text);

/* info */
lk_bool_t *lk_file_isdirectory(lk_file_t *self);
lk_bool_t *lk_file_isexecutable(lk_file_t *self);
lk_bool_t *lk_file_isexists(lk_file_t *self);
lk_str_t *lk_file_read_num(lk_file_t *self, lk_num_t *length);
lk_str_t *lk_file_readall(lk_file_t *self);
lk_str_t *lk_file_readuntil_char(lk_file_t *self, lk_char_t *until);
lk_str_t *lk_file_readuntil_charset(lk_file_t *self, lk_charset_t *until);
lk_bool_t *lk_file_isreadable(lk_file_t *self);
lk_num_t *lk_file_size(lk_file_t *self);
lk_bool_t *lk_file_iswritable(lk_file_t *self);
#endif
