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
} list_t;

/* new */
list_t *list_alloc(int ilen, int capa);
list_t *list_allocptr(void);
list_t *list_allocptrwithcapa(int capa);
list_t *list_allocfromfile(FILE *stream, size_t rs);
list_t *list_clone(list_t *self);
void list_init(list_t *self, int ilen, int capa);
void list_initptr(list_t *self);
void list_copy(list_t *self, list_t *src);
void list_fin(list_t *self);
void list_free(list_t *self);

/* update */
void list_clear(list_t *self);
void list_concat(list_t *self, list_t *v);
void list_insert(list_t *self, int i, void *v);
void list_insertptr(list_t *self, int i, void *v);
void list_insertuchar(list_t *self, int i, uint32_t v);
void list_limit(list_t *self, int n);
void list_offset(list_t *self, int n);
void *list_peekptr(list_t *self);
uint32_t list_peekuchar(list_t *self);
void *list_popptr(list_t *self);
uint32_t list_popuchar(list_t *self);
void list_pushptr(list_t *self, void *v);
void list_pushuchar(list_t *self, uint32_t v);
void list_remove(list_t *self, int i);
void *list_removeptr(list_t *self, int i);
uint32_t list_removeuchar(list_t *self, int i);
void list_resize(list_t *self, int s);
void list_resizeitem(list_t *self, list_t *other);
void list_reverse(list_t *self);
void list_set(list_t *self, int i, void *v);
void list_setptr(list_t *self, int i, void *v);
void list_setrange(list_t *self, int b, int e, list_t *v);
void list_setuchar(list_t *self, int i, uint32_t v);
void *list_shiftptr(list_t *self);
void list_slice(list_t *self, int offset, int limit);
const char *list_tocstr(list_t *self);
void list_unshiftptr(list_t *self, void *v);

/* info */
#define LIST_AT(self, i) ((self)->first + (self)->data->ilen * i)
#define LIST_ATPTR(self, i) (*(void **)LIST_AT(self, (i)))
int list_cmp(const list_t *self, const list_t *other);
int list_cmpcstr(const list_t *self, const char *other);
#define LIST_COUNT(self) ((self)->count)
#define LIST_EACH(self, i, v, block) do { \
    list_t *_l = (self); \
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
    ? 0 : list_cmp((self), (other)) == 0 \
)
#include "cset.h"
int list_findcset(const list_t *self, const cset_t *pat, int o);
int list_findlist(const list_t *self, const list_t *pat, int o);
int list_finduchar(const list_t *self, uint32_t pat, int o);
void *list_get(const list_t *self, int i);
void *list_getptr(const list_t *self, int i);
uint32_t list_getuchar(const list_t *self, int i);
int list_hc(const list_t *self);
#define LIST_ISINIT(self) ((self)->data != NULL)
void list_write(const list_t *self, FILE *stream);
#endif
