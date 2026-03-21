#ifndef DARRAY_H
#define DARRAY_H
#include "common.h"

/* type - contains actual list data */
struct listdata {
    int  cap;
    int  used;
    int  ilen; /* length of item in data in bytes */
    int  refc; /* how many ref this buf? */
    char item; /* placeholder - first item */
};

/* type */
typedef struct list {
    struct listdata *data;
    char               *first;
    int                 size;
} darray_t;

/* new */
darray_t *darray_alloc(int ilen, int cap);
darray_t *darray_clone(darray_t *self);
void darray_copy(darray_t *self, darray_t *src);
void darray_init(darray_t *self, int ilen, int cap);
void darray_fin(darray_t *self);
void darray_free(darray_t *self);

/* update */
void darray_clear(darray_t *self);
void darray_concat(darray_t *self, darray_t *v);
void darray_insert(darray_t *self, int i, void *v);
void darray_limit(darray_t *self, int n);
void darray_offset(darray_t *self, int n);
void darray_remove(darray_t *self, int i);
void darray_resize(darray_t *self, int s);
void darray_resizeitem(darray_t *self, darray_t *other);
void darray_reverse(darray_t *self);
void darray_set(darray_t *self, int i, void *v);
void darray_setrange(darray_t *self, int b, int e, darray_t *v);
void darray_slice(darray_t *self, int offset, int limit);

/* info */
#define DARRAY_AT(self, i) ((self)->first + (self)->data->ilen * i)
#define DARRAY_ATPTR(self, i) (*(void **)DARRAY_AT(self, (i)))
int darray_cmp(const darray_t *self, const darray_t *other);
#define DARRAY_COUNT(self) ((self)->size)
#define DARRAY_EACH(self, i, v, block) do { \
    darray_t *_l = (self); \
    int i, _c = DARRAY_COUNT(_l); \
    void *v; \
    for(i = 0; i < _c; i ++) { \
        v = DARRAY_AT(_l, i); \
        { block; } \
    } \
} while(0)
#define DARRAY_EACHPTR(self, i, v, block) \
    DARRAY_EACH(self, i, v, v = *(void **)v; block);
#define DARRAY_EQ(self, other) ( \
    DARRAY_COUNT(self) != DARRAY_COUNT(other) \
    ? 0 : darray_cmp((self), (other)) == 0 \
)
int darray_find_darray(const darray_t *self, const darray_t *pat, int o);
void *darray_get(const darray_t *self, int i);
int darray_hc(const darray_t *self);
#define DARRAY_ISINIT(self) ((self)->data != NULL)
void darray_write(const darray_t *self, FILE *stream);
void darray_print_tostream(const darray_t *self, FILE *stream);

/* string */
/* new */
darray_t *darray_str_alloc(void);
darray_t *darray_str_alloc_fromcstr(const char *cstr);
darray_t *darray_str_alloc_fromdata(const void *data, int len);
darray_t *darray_str_alloc_fromfile(FILE *stream);
darray_t *darray_str_alloc_fromfile_untilchar(FILE *stream, uint32_t pat);
darray_t *darray_str_alloc_fromfile_withsize(FILE *stream, size_t rs);

/* update */
void darray_str_insert(darray_t *self, int i, uint32_t v);
uint32_t darray_str_pop(darray_t *self);
void darray_str_push(darray_t *self, uint32_t v);
uint32_t darray_str_remove(darray_t *self, int i);
void darray_str_set(darray_t *self, int i, uint32_t v);

/* info */
uint32_t darray_str_peek(darray_t *self);
const char *darray_str_tocstr(darray_t *self);
int darray_str_cmp_cstr(const darray_t *self, const char *other);
int darray_str_find(const darray_t *self, uint32_t pat, int o);
uint32_t darray_str_get(const darray_t *self, int i);

/* ptr list */
/* new */
darray_t *darray_ptr_alloc(void);
darray_t *darray_ptr_alloc_withcap(int cap);
void darray_ptr_init(darray_t *self);

/* update */
void darray_ptr_insert(darray_t *self, int i, void *v);
void *darray_ptr_pop(darray_t *self);
void darray_ptr_push(darray_t *self, void *v);
void *darray_ptr_remove(darray_t *self, int i);
void darray_ptr_set(darray_t *self, int i, void *v);
void *darray_ptr_shift(darray_t *self);
void darray_ptr_unshift(darray_t *self, void *v);

/* info */
void *darray_ptr_peek(darray_t *self);
void *darray_ptr_get(const darray_t *self, int i);

/* charset-related */
#include "charset.h"
darray_t *darray_str_alloc_fromfile_untilcharset(FILE *stream, const charset_t *pat);
int darray_str_findset(const darray_t *self, const charset_t *pat, int o);
#endif
