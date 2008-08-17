#ifndef STRING_H
#define STRING_H
#include "common.h"
#include "array.h"

/* new */
array_t *array_allocFromData(const void *data, int len);
array_t *array_allocFromCString(const char *cstr);
array_t *string_allocfromfile(FILE *stream);
array_t *array_allocFromFileUntilChar(FILE *stream, uint32_t pat);
array_t *array_allocFromFileUntilCSet(FILE *stream, const charset_t *pat);

/* info */
void string_print(const array_t *self, FILE *stream);
#endif
