#ifndef VEC_H
#define VEC_H
#include "common.h"

#pragma region vec

// Heap-allocated backing buffer, ref-counted and shared across slices
struct vec_buf {
    int capacity;
    int used;      // high-water mark used for copy-on-write decisions
    int item_size; // size of each item in bytes
    int ref_count; // number of vecs sharing this buf
    char items[];  // inline data
};

typedef struct vec {
    struct vec_buf *buf;
    char *first;
    int length;
} vec_t;

vec_t *vec_alloc(int item_size, int capacity);
vec_t *vec_clone(vec_t *self);
void vec_copy(vec_t *self, vec_t *src);
void vec_init(vec_t *self, int item_size, int capacity);
void vec_fin(vec_t *self);
void vec_free(vec_t *self);

void vec_clear(vec_t *self);
void vec_concat(vec_t *self, vec_t *other);
void vec_insert(vec_t *self, int at, void *value);
void vec_limit(vec_t *self, int limit);
void vec_offset(vec_t *self, int offset);
void vec_remove(vec_t *self, int at);
void vec_resize(vec_t *self, int length);
void vec_resize_item(vec_t *self, vec_t *other);
void vec_reverse(vec_t *self);
void vec_set(vec_t *self, int at, void *value);
void vec_set_range(vec_t *self, int start, int end, vec_t *other);
void vec_slice(vec_t *self, int offset, int limit);

#define VEC_AT(self, i) ((self)->first + (self)->buf->item_size * i)
#define VEC_ATPTR(self, i) (*(void **)VEC_AT(self, (i)))
#define VEC_COUNT(self) ((self)->length)
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
#define VEC_ISINIT(self) ((self)->buf != NULL)
int vec_cmp(const vec_t *self, const vec_t *other);
int vec_find_vec(const vec_t *self, const vec_t *pattern, int offset);
void *vec_get(const vec_t *self, int at);
int vec_hc(const vec_t *self);
void vec_write(const vec_t *self, FILE *stream);
void vec_print_tostream(const vec_t *self, FILE *stream);

#pragma endregion
#pragma region string
vec_t *vec_str_alloc(void);
vec_t *vec_str_alloc_from_cstr(const char *cstr);
vec_t *vec_str_alloc_from_data(const void *data, int length);
vec_t *vec_str_alloc_from_file(FILE *stream);
vec_t *vec_str_alloc_from_file_until_char(FILE *stream, uint32_t pattern);
vec_t *vec_str_alloc_from_file_with_length(FILE *stream, size_t length);

void vec_str_insert(vec_t *self, int at, uint32_t value);
uint32_t vec_str_pop(vec_t *self);
void vec_str_push(vec_t *self, uint32_t value);
uint32_t vec_str_remove(vec_t *self, int at);
void vec_str_set(vec_t *self, int at, uint32_t value);

uint32_t vec_str_peek(vec_t *self);
const char *vec_str_tocstr(vec_t *self);
int vec_str_cmp_cstr(const vec_t *self, const char *other);
int vec_str_find(const vec_t *self, uint32_t pattern, int offset);
uint32_t vec_str_get(const vec_t *self, int at);

// charset-dependent — included here due to circular dependency between vec.h and charset.h
#include "charset.h"
vec_t *vec_str_alloc_from_file_until_charset(FILE *stream, const charset_t *pattern);
int vec_str_find_charset(const vec_t *self, const charset_t *pattern, int offset);

#pragma endregion
#pragma region ptr list
vec_t *vec_ptr_alloc(void);
vec_t *vec_ptr_alloc_with_capacity(int capacity);
void vec_ptr_init(vec_t *self);

void vec_ptr_insert(vec_t *self, int at, void *value);
void *vec_ptr_pop(vec_t *self);
void vec_ptr_push(vec_t *self, void *value);
void *vec_ptr_remove(vec_t *self, int at);
void vec_ptr_set(vec_t *self, int at, void *value);
void *vec_ptr_shift(vec_t *self);
void vec_ptr_unshift(vec_t *self, void *value);

void *vec_ptr_peek(vec_t *self);
void *vec_ptr_get(const vec_t *self, int at);

#pragma endregion
#endif
