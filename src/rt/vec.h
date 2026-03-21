#ifndef VEC_H
#define VEC_H
#include "common.h"

// type - contains actual list data
struct vec_buf {
    int capacity;
    int used;
    int item_size; // length of item in bytes
    int ref_count; // how many darrays share this buf?
    char item;     // placeholder - first item
};

// type
typedef struct vec {
    struct vec_buf *buf;
    char *first;
    int size;
} vec_t;

// new
vec_t *vec_alloc(int ilen, int cap);
vec_t *vec_clone(vec_t *self);
void vec_copy(vec_t *self, vec_t *src);
void vec_init(vec_t *self, int ilen, int cap);
void vec_fin(vec_t *self);
void vec_free(vec_t *self);

// update
void vec_clear(vec_t *self);
void vec_concat(vec_t *self, vec_t *v);
void vec_insert(vec_t *self, int i, void *v);
void vec_limit(vec_t *self, int n);
void vec_offset(vec_t *self, int n);
void vec_remove(vec_t *self, int i);
void vec_resize(vec_t *self, int s);
void vec_resizeitem(vec_t *self, vec_t *other);
void vec_reverse(vec_t *self);
void vec_set(vec_t *self, int i, void *v);
void vec_setrange(vec_t *self, int b, int e, vec_t *v);
void vec_slice(vec_t *self, int offset, int limit);

// info
#define VEC_AT(self, i) ((self)->first + (self)->buf->item_size * i)
#define VEC_ATPTR(self, i) (*(void **)VEC_AT(self, (i)))
int vec_cmp(const vec_t *self, const vec_t *other);
#define VEC_COUNT(self) ((self)->size)
#define VEC_EACH(self, i, v, block) \
    do { \
        vec_t *_l = (self); \
        int i, _c = VEC_COUNT(_l); \
        void *v; \
        for (i = 0; i < _c; i++) { \
            v = VEC_AT(_l, i); \
            { \
                block; \
            } \
        } \
    } while (0)
#define VEC_EACH_PTR(self, i, v, block) VEC_EACH(self, i, v, v = *(void **)v; block);
#define VEC_EQ(self, other) (VEC_COUNT(self) != VEC_COUNT(other) ? 0 : vec_cmp((self), (other)) == 0)
int vec_find_darray(const vec_t *self, const vec_t *pat, int o);
void *vec_get(const vec_t *self, int i);
int vec_hc(const vec_t *self);
#define VEC_ISINIT(self) ((self)->buf != NULL)
void vec_write(const vec_t *self, FILE *stream);
void vec_print_tostream(const vec_t *self, FILE *stream);

// string
// new
vec_t *vec_str_alloc(void);
vec_t *vec_str_alloc_fromcstr(const char *cstr);
vec_t *vec_str_alloc_fromdata(const void *data, int len);
vec_t *vec_str_alloc_fromfile(FILE *stream);
vec_t *vec_str_alloc_fromfile_untilchar(FILE *stream, uint32_t pat);
vec_t *vec_str_alloc_fromfile_withsize(FILE *stream, size_t rs);

// update
void vec_str_insert(vec_t *self, int i, uint32_t v);
uint32_t vec_str_pop(vec_t *self);
void vec_str_push(vec_t *self, uint32_t v);
uint32_t vec_str_remove(vec_t *self, int i);
void vec_str_set(vec_t *self, int i, uint32_t v);

// info
uint32_t vec_str_peek(vec_t *self);
const char *vec_str_tocstr(vec_t *self);
int vec_str_cmp_cstr(const vec_t *self, const char *other);
int vec_str_find(const vec_t *self, uint32_t pat, int o);
uint32_t vec_str_get(const vec_t *self, int i);

// ptr list
// new
vec_t *vec_ptr_alloc(void);
vec_t *vec_ptr_alloc_withcap(int cap);
void vec_ptr_init(vec_t *self);

// update
void vec_ptr_insert(vec_t *self, int i, void *v);
void *vec_ptr_pop(vec_t *self);
void vec_ptr_push(vec_t *self, void *v);
void *vec_ptr_remove(vec_t *self, int i);
void vec_ptr_set(vec_t *self, int i, void *v);
void *vec_ptr_shift(vec_t *self);
void vec_ptr_unshift(vec_t *self, void *v);

// info
void *vec_ptr_peek(vec_t *self);
void *vec_ptr_get(const vec_t *self, int i);

// charset-related
#include "charset.h"
vec_t *vec_str_alloc_fromfile_untilcharset(FILE *stream, const charset_t *pat);
int vec_str_findset(const vec_t *self, const charset_t *pat, int o);
#endif
