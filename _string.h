#ifndef STRING_H
#define STRING_H
#include "_common.h"
#include "_list.h"

/* type */
#define string_t list_t

/* new */
string_t *string_alloc(uint8_t ilen, int capa);
string_t *string_allocfromdata(const void *data, int len);
string_t *string_allocfromcstr(const char *cstr);
string_t *string_allocfromfile(FILE *stream);
string_t *string_allocfromfileuntilchar(FILE *stream, uint32_t pat);
string_t *string_allocfromfileuntilcset(FILE *stream, const cset_t *pat);

/* info */
void string_print(const string_t *self, FILE *stream);
#endif
