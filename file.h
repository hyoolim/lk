#ifndef LK_FILE_H
#define LK_FILE_H
#include "types.h"

/* type */
struct lk_file {
    struct lk_common  o;
    lk_string_t      *path;
    FILE             *file;
};

/* init */
void lk_file_typeinit(lk_vm_t *vm);
void lk_file_libinit(lk_vm_t *vm);

/* update */
void lk_file_close(lk_object_t *self);
void lk_file_delete(lk_object_t *self);
void lk_file_flush(lk_object_t *self);
void lk_file_init_string(lk_object_t *self, lk_string_t *path);
void lk_file_move_string(lk_object_t *self, lk_string_t *dest);
void lk_file_openforreading(lk_object_t *self);
void lk_file_openforwriting(lk_object_t *self);
void lk_file_write_string(lk_object_t *self, lk_string_t *text);

/* info */
lk_bool_t *lk_file_isdirectory(lk_object_t *self);
lk_bool_t *lk_file_isexecutable(lk_object_t *self);
lk_bool_t *lk_file_isexists(lk_object_t *self);
lk_string_t *lk_file_read_number(lk_object_t *self, lk_number_t *length);
lk_string_t *lk_file_readall(lk_object_t *self);
lk_string_t *lk_file_readuntil_char(lk_object_t *self, lk_char_t *until);
lk_string_t *lk_file_readuntil_charset(lk_object_t *self, lk_charset_t *until);
lk_bool_t *lk_file_isreadable(lk_object_t *self);
lk_number_t *lk_file_size(lk_object_t *self);
lk_bool_t *lk_file_iswritable(lk_object_t *self);
#endif
