#ifndef LIST_H
#define LIST_H
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
darray_t *darray_allocptr(void);
darray_t *darray_allocptrwithcap(int cap);
darray_t *darray_allocfromfile(FILE *stream, size_t rs);
darray_t *darray_alloc_fromdata(const void *data, int len);
darray_t *darray_allocFromCString(const char *cstr);
darray_t *str_allocfromfile(FILE *stream);
darray_t *darray_alloc_fromfile_untilchar(FILE *stream, uint32_t pat);
darray_t *darray_alloc_str(void);
darray_t *darray_clone(darray_t *self);
void darray_init(darray_t *self, int ilen, int cap);
void darray_initptr(darray_t *self);
void darray_copy(darray_t *self, darray_t *src);
void darray_fin(darray_t *self);
void darray_free(darray_t *self);

/* update */
void darray_clear(darray_t *self);
void darray_concat(darray_t *self, darray_t *v);
void darray_insert(darray_t *self, int i, void *v);
void darray_insertptr(darray_t *self, int i, void *v);
void darray_insertuchar(darray_t *self, int i, uint32_t v);
void darray_limit(darray_t *self, int n);
void darray_offset(darray_t *self, int n);
void *darray_peekptr(darray_t *self);
uint32_t darray_peekuchar(darray_t *self);
void *darray_popptr(darray_t *self);
uint32_t darray_popuchar(darray_t *self);
void darray_pushptr(darray_t *self, void *v);
void darray_pushuchar(darray_t *self, uint32_t v);
void darray_remove(darray_t *self, int i);
void *darray_removeptr(darray_t *self, int i);
uint32_t darray_removeuchar(darray_t *self, int i);
void darray_resize(darray_t *self, int s);
void darray_resizeitem(darray_t *self, darray_t *other);
void darray_reverse(darray_t *self);
void darray_set(darray_t *self, int i, void *v);
void darray_setptr(darray_t *self, int i, void *v);
void darray_setrange(darray_t *self, int b, int e, darray_t *v);
void darray_setuchar(darray_t *self, int i, uint32_t v);
void *darray_shiftptr(darray_t *self);
void darray_slice(darray_t *self, int offset, int limit);
const char *darray_tocstr(darray_t *self);
void darray_unshiftptr(darray_t *self, void *v);

/* info */
#define LIST_AT(self, i) ((self)->first + (self)->data->ilen * i)
#define LIST_ATPTR(self, i) (*(void **)LIST_AT(self, (i)))
int darray_compareTo(const darray_t *self, const darray_t *other);
int darray_cmp_tocstr(const darray_t *self, const char *other);
#define LIST_COUNT(self) ((self)->size)
#define LIST_EACH(self, i, v, block) do { \
    darray_t *_l = (self); \
    int i, _c = LIST_COUNT(_l); \
    void *v; \
    for(i = 0; i < _c; i ++) { \
        v = LIST_AT(_l, i); \
        { block; } \
    } \
} while(0)
#define LIST_EACHPTR(self, i, v, block) \
    LIST_EACH(self, i, v, v = *(void **)v; block);
#define LIST_EQ(self, other) ( \
    LIST_COUNT(self) != LIST_COUNT(other) \
    ? 0 : darray_compareTo((self), (other)) == 0 \
)
#include "charset.h"
darray_t *darray_alloc_fromfile_untilcharset(FILE *stream, const charset_t *pat);
int darray_find_charset(const darray_t *self, const charset_t *pat, int o);
int darray_find_darray(const darray_t *self, const darray_t *pat, int o);
int darray_find_char(const darray_t *self, uint32_t pat, int o);
void *darray_get(const darray_t *self, int i);
void *darray_getptr(const darray_t *self, int i);
uint32_t darray_getuchar(const darray_t *self, int i);
int darray_hc(const darray_t *self);
#define LIST_ISINIT(self) ((self)->data != NULL)
void darray_write(const darray_t *self, FILE *stream);
void darray_print_tostream(const darray_t *self, FILE *stream);
#endif
