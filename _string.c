#include "_string.h"

/* new */
string_t *string_alloc(uint8_t ilen, int capa) {
    return (string_t *)list_alloc(ilen, capa);
}
string_t *string_allocfromdata(const void *data, int len) {
    string_t *self = string_alloc(sizeof(uint8_t), len);
    memcpy(self->first, data, len);
    self->data->used = self->count = len;
    return self;
}
string_t *string_allocfromcstr(const char *cstr) {
    return string_allocfromdata(cstr, strlen(cstr));
}
string_t *string_allocfromfile(FILE *stream) {
    if(fseek(stream, 0, SEEK_END) == 0) {
        long size;
        if((size = ftell(stream)) >= 0
        && fseek(stream, 0, SEEK_SET) == 0) {
            string_t *self = string_alloc(sizeof(uint8_t), size);
            fread(self->first, 1, size, stream);
            self->count = self->data->used = size;
            return self;
    }   }
    return NULL;
}
#define UNTIL(check) do { \
    int i, c = 64; \
    string_t *self = string_alloc(sizeof(uint8_t), c); \
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
string_t *string_allocfromfileuntilchar(FILE *stream, uint32_t pat) {
    if(feof(stream)) return NULL;
    UNTIL(ch == pat);
}
string_t *string_allocfromfileuntilcset(FILE *stream, const cset_t *pat) {
    if(feof(stream)) return NULL;
    UNTIL(cset_has(pat, ch));
}

/* info */
void string_print(const string_t *self, FILE *stream) {
    fwrite(self->first, self->data->ilen, self->count, stream);
}
