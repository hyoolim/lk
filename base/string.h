#ifndef STRING_H
#define STRING_H
#include "common.h"
#include "list.h"

/* new */
list_t *string_allocfromdata(const void *data, int len);
list_t *string_allocfromcstr(const char *cstr);
list_t *string_allocfromfile(FILE *stream);
list_t *string_allocfromfileuntilchar(FILE *stream, uint32_t pat);
list_t *string_allocfromfileuntilcset(FILE *stream, const cset_t *pat);

/* info */
void string_print(const list_t *self, FILE *stream);
#endif
