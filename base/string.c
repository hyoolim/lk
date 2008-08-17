#include "string.h"

/* new */
array_t *array_allocFromData(const void *data, int len) {
    array_t *self = array_alloc(sizeof(uint8_t), len);
    memcpy(self->first, data, len);
    self->data->used = self->count = len;
    return self;
}
array_t *array_allocFromCString(const char *cstr) {
    return array_allocFromData(cstr, strlen(cstr));
}
array_t *string_allocfromfile(FILE *stream) {
    if(fseek(stream, 0, SEEK_END) == 0) {
        long size;
        if((size = ftell(stream)) >= 0
        && fseek(stream, 0, SEEK_SET) == 0) {
            array_t *self = array_alloc(sizeof(uint8_t), size);
            fread(self->first, 1, size, stream);
            self->count = self->data->used = size;
            return self;
    }   }
    return NULL;
}
#define UNTIL(check) do { \
    int i, c = 64; \
    array_t *self = array_alloc(sizeof(uint8_t), c); \
    int ch; \
    for(i = 0; (ch = fgetc(stream)) != EOF; i ++) { \
        if(i >= c) { \
            do { c *= 2; } while(i >= c); \
            self->data = memory_resize(self->data, \
            sizeof(struct listdata) - sizeof(char) \
            + self->data->ilen * c); \
            self->data->capa = (c); \
            self->first = &(self)->data->item; \
        } \
        ((char *)self->first)[i] = ch; \
        if((check)) break; \
    } \
    self->count = self->data->used = i + 1; \
    return self; \
} while(0)
array_t *array_allocFromFileUntilChar(FILE *stream, uint32_t pat) {
    if(feof(stream)) return NULL;
    UNTIL(ch == pat);
}
array_t *array_allocFromFileUntilCSet(FILE *stream, const charset_t *pat) {
    if(feof(stream)) return NULL;
    UNTIL(charset_has(pat, ch));
}

/* info */
void string_print(const array_t *self, FILE *stream) {
    fwrite(self->first, self->data->ilen, self->count, stream);
}
