#ifndef LK_FILE_H
#define LK_FILE_H
#include "types.h"

// type
struct lk_file {
    struct lk_common o;
    lk_str_t *path;
    lk_str_t *name;
    FILE *fd;
};

// init
void lk_file_type_init(lk_vm_t *vm);
void lk_file_lib_init(lk_vm_t *vm);

// new
lk_file_t *lk_file_new_with_path(lk_vm_t *vm, lk_str_t *path);

// update
void lk_file_close(lk_file_t *self);
void lk_file_delete(lk_file_t *self);
void lk_file_flush(lk_file_t *self);
void lk_file_init_str(lk_file_t *self, lk_str_t *path);
void lk_file_move_str(lk_file_t *self, lk_str_t *dest);
void lk_file_open(lk_file_t *self, lk_str_t *mode);
void lk_file_write_str(lk_file_t *self, lk_str_t *text);

// info
lk_bool_t *lk_file_is_directory(lk_file_t *self);
lk_bool_t *lk_file_is_executable(lk_file_t *self);
lk_bool_t *lk_file_is_exists(lk_file_t *self);
lk_str_t *lk_file_read_num(lk_file_t *self, lk_num_t *length);
lk_str_t *lk_file_read_all(lk_file_t *self);
lk_str_t *lk_file_read_until_char(lk_file_t *self, lk_char_t *until);
lk_str_t *lk_file_read_until_charset(lk_file_t *self, lk_charset_t *until);
lk_bool_t *lk_file_is_readable(lk_file_t *self);
lk_num_t *lk_file_size(lk_file_t *self);
lk_bool_t *lk_file_is_writable(lk_file_t *self);
#endif
