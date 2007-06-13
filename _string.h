#ifndef PT_STRING_H
#define PT_STRING_H
#include "_common.h"
#include "_list.h"

/* type */
#define pt_string_t pt_list_t
#define PT_STRING(o) ((pt_string_t *)(o))

/* new */
pt_string_t *pt_string_alloc(uint8_t ilen, int capa);
pt_string_t *pt_string_allocfromdata(const void *data, int len);
pt_string_t *pt_string_allocfromcstr(const char *cstr);
pt_string_t *pt_string_allocfromfile(FILE *stream);
pt_string_t *pt_string_allocfromfileuntilchar(FILE *stream, uint32_t pat);
pt_string_t *pt_string_allocfromfileuntilcset(FILE *stream, const pt_cset_t *pat);

/* info */
void pt_string_print(const pt_string_t *self, FILE *stream);
#endif
