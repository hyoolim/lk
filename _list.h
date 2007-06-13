#ifndef PT_LIST_H
#define PT_LIST_H
#include "_common.h"

/* type - contains actual list data */
struct pt_listdata {
    int  capa;
    int  used;
    int  ilen; /* length of item in data in bytes */
    int  refc; /* how many ref this buf? */
    char item; /* placeholder - first item */
};

/* type */
typedef struct pt_list {
    struct pt_listdata *data;
    char               *first;
    int                 count;
} pt_list_t;
#define PT_LIST(o) ((pt_list_t *)(o))

/* new */
pt_list_t *pt_list_alloc(int ilen, int capa);
pt_list_t *pt_list_allocptr(void);
pt_list_t *pt_list_allocptrwithcapa(int capa);
pt_list_t *pt_list_allocfromfile(FILE *stream, size_t rs);
pt_list_t *pt_list_clone(pt_list_t *self);
void pt_list_init(pt_list_t *self, int ilen, int capa);
void pt_list_initptr(pt_list_t *self);
void pt_list_copy(pt_list_t *self, pt_list_t *src);
void pt_list_fin(pt_list_t *self);
void pt_list_free(pt_list_t *self);

/* update */
void pt_list_clear(pt_list_t *self);
void pt_list_concat(pt_list_t *self, pt_list_t *v);
void pt_list_insert(pt_list_t *self, int i, void *v);
void pt_list_insertptr(pt_list_t *self, int i, void *v);
void pt_list_insertuchar(pt_list_t *self, int i, uint32_t v);
void pt_list_limit(pt_list_t *self, int n);
void pt_list_offset(pt_list_t *self, int n);
void *pt_list_peekptr(pt_list_t *self);
uint32_t pt_list_peekuchar(pt_list_t *self);
void *pt_list_popptr(pt_list_t *self);
uint32_t pt_list_popuchar(pt_list_t *self);
void pt_list_pushptr(pt_list_t *self, void *v);
void pt_list_pushuchar(pt_list_t *self, uint32_t v);
void pt_list_remove(pt_list_t *self, int i);
void *pt_list_removeptr(pt_list_t *self, int i);
uint32_t pt_list_removeuchar(pt_list_t *self, int i);
void pt_list_resize(pt_list_t *self, int s);
void pt_list_resizeitem(pt_list_t *self, pt_list_t *other);
void pt_list_reverse(pt_list_t *self);
void pt_list_set(pt_list_t *self, int i, void *v);
void pt_list_setptr(pt_list_t *self, int i, void *v);
void pt_list_setrange(pt_list_t *self, int b, int e, pt_list_t *v);
void pt_list_setuchar(pt_list_t *self, int i, uint32_t v);
void *pt_list_shiftptr(pt_list_t *self);
void pt_list_slice(pt_list_t *self, int offset, int limit);
const char *pt_list_tocstr(pt_list_t *self);
void pt_list_unshiftptr(pt_list_t *self, void *v);

/* info */
#define PT_LIST_AT(self, i) ((self)->first + (self)->data->ilen * i)
#define PT_LIST_ATPTR(self, i) (*(void **)PT_LIST_AT(self, (i)))
int pt_list_cmp(const pt_list_t *self, const pt_list_t *other);
int pt_list_cmpcstr(const pt_list_t *self, const char *other);
#define PT_LIST_COUNT(self) ((self)->count)
#define PT_LIST_EACH(self, i, v, block) do { \
    pt_list_t *_l = (self); \
    int i, _c = PT_LIST_COUNT(_l); \
    void *v; \
    for(i = 0; i < _c; i ++) { \
        v = PT_LIST_AT(_l, i); \
        { block; } \
    } \
} while(0)
#define PT_LIST_EACHPTR(self, i, v, block) \
    PT_LIST_EACH(self, i, v, v = *(void **)v; block);
#define PT_LIST_EQ(self, other) ( \
    PT_LIST_COUNT(self) != PT_LIST_COUNT(other) \
    ? 0 : pt_list_cmp((self), (other)) == 0 \
)
#include "_cset.h"
int pt_list_findcset(const pt_list_t *self, const pt_cset_t *pat, int o);
int pt_list_findlist(const pt_list_t *self, const pt_list_t *pat, int o);
int pt_list_finduchar(const pt_list_t *self, uint32_t pat, int o);
void *pt_list_get(const pt_list_t *self, int i);
void *pt_list_getptr(const pt_list_t *self, int i);
uint32_t pt_list_getuchar(const pt_list_t *self, int i);
int pt_list_hc(const pt_list_t *self);
#define PT_LIST_ISINIT(self) ((self)->data != NULL)
void pt_list_write(const pt_list_t *self, FILE *stream);
#endif
