#ifndef STRING_H
#define STRING_H
#include "common.h"
#include "list.h"

/* new */
Sequence_t *string_allocfromdata(const void *data, int len);
Sequence_t *string_allocfromcstr(const char *cstr);
Sequence_t *string_allocfromfile(FILE *stream);
Sequence_t *string_allocfromfileuntilchar(FILE *stream, uint32_t pat);
Sequence_t *string_allocfromfileuntilcset(FILE *stream, const CharacterSet_t *pat);

/* info */
void string_print(const Sequence_t *self, FILE *stream);
#endif
