#ifndef LIST_H
#define LIST_H
#include "common.h"

/* type - contains actual list data */
struct listdata {
    int  capa;
    int  used;
    int  ilen; /* length of item in data in bytes */
    int  refc; /* how many ref this buf? */
    char item; /* placeholder - first item */
};

/* type */
typedef struct list {
    struct listdata *data;
    char               *first;
    int                 count;
} array_t;

/* new */
array_t *array_alloc(int ilen, int capa);
array_t *array_allocptr(void);
array_t *array_allocptrwithcapa(int capa);
array_t *array_allocfromfile(FILE *stream, size_t rs);
array_t *array_clone(array_t *self);
void array_init(array_t *self, int ilen, int capa);
void array_initptr(array_t *self);
void array_copy(array_t *self, array_t *src);
void array_fin(array_t *self);
void array_free(array_t *self);

/* update */
void array_clear(array_t *self);
void array_concat(array_t *self, array_t *v);
void array_insert(array_t *self, int i, void *v);
void array_insertptr(array_t *self, int i, void *v);
void array_insertuchar(array_t *self, int i, uint32_t v);
void array_limit(array_t *self, int n);
void array_offset(array_t *self, int n);
void *array_peekptr(array_t *self);
uint32_t array_peekuchar(array_t *self);
void *array_popptr(array_t *self);
uint32_t array_popuchar(array_t *self);
void array_pushptr(array_t *self, void *v);
void array_pushuchar(array_t *self, uint32_t v);
void array_remove(array_t *self, int i);
void *array_removeptr(array_t *self, int i);
uint32_t array_removeuchar(array_t *self, int i);
void array_resize(array_t *self, int s);
void array_resizeitem(array_t *self, array_t *other);
void array_reverse(array_t *self);
void array_set(array_t *self, int i, void *v);
void array_setptr(array_t *self, int i, void *v);
void array_setrange(array_t *self, int b, int e, array_t *v);
void array_setuchar(array_t *self, int i, uint32_t v);
void *array_shiftptr(array_t *self);
void array_slice(array_t *self, int offset, int limit);
const char *array_tocstr(array_t *self);
void array_unshiftptr(array_t *self, void *v);

/* info */
#define LIST_AT(self, i) ((self)->first + (self)->data->ilen * i)
#define LIST_ATPTR(self, i) (*(void **)LIST_AT(self, (i)))
int array_cmp(const array_t *self, const array_t *other);
int array_cmpcstr(const array_t *self, const char *other);
#define LIST_COUNT(self) ((self)->count)
#define LIST_EACH(self, i, v, block) do { \
    array_t *_l = (self); \
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
    ? 0 : array_cmp((self), (other)) == 0 \
)
#include "charset.h"
int array_findCharset(const array_t *self, const charset_t *pat, int o);
int array_findlist(const array_t *self, const array_t *pat, int o);
int array_finduchar(const array_t *self, uint32_t pat, int o);
void *array_get(const array_t *self, int i);
void *array_getptr(const array_t *self, int i);
uint32_t array_getuchar(const array_t *self, int i);
int array_hc(const array_t *self);
#define LIST_ISINIT(self) ((self)->data != NULL)
void array_write(const array_t *self, FILE *stream);
#endif
