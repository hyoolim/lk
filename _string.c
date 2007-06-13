#include "_string.h"

/* new */
pt_string_t *pt_string_alloc(uint8_t ilen, int capa) {
    return PT_STRING(pt_list_alloc(ilen, capa));
}
pt_string_t *pt_string_allocfromdata(const void *data, int len) {
    pt_string_t *self = pt_string_alloc(sizeof(uint8_t), len);
    memcpy(self->first, data, len);
    self->data->used = self->count = len;
    return self;
}
pt_string_t *pt_string_allocfromcstr(const char *cstr) {
    return pt_string_allocfromdata(cstr, strlen(cstr));
}
pt_string_t *pt_string_allocfromfile(FILE *stream) {
    if(fseek(stream, 0, SEEK_END) == 0) {
        long size;
        if((size = ftell(stream)) >= 0
        && fseek(stream, 0, SEEK_SET) == 0) {
            pt_string_t *self = pt_string_alloc(sizeof(uint8_t), size);
            fread(self->first, 1, size, stream);
            self->count = self->data->used = size;
            return self;
    }   }
    return NULL;
}
#define UNTIL(check) do { \
    int i, c = 64; \
    pt_string_t *self = pt_string_alloc(sizeof(uint8_t), c); \
    int ch; \
    for(i = 0; (ch = fgetc(stream)) != EOF; i ++) { \
        if(i >= c) { \
            do { c *= 2; } while(i >= c); \
            self->data = pt_memory_resize(self->data, \
            sizeof(struct pt_listdata) - sizeof(char) \
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
pt_string_t *pt_string_allocfromfileuntilchar(FILE *stream, uint32_t pat) {
    if(feof(stream)) return NULL;
    UNTIL(ch == pat);
}
pt_string_t *pt_string_allocfromfileuntilcset(FILE *stream, const pt_cset_t *pat) {
    if(feof(stream)) return NULL;
    UNTIL(pt_cset_has(pat, ch));
}

/* info */
void pt_string_print(const pt_string_t *self, FILE *stream) {
    fwrite(self->first, self->data->ilen, self->count, stream);
}
