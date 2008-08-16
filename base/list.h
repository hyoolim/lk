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
} Sequence_t;

/* new */
Sequence_t *Sequence_alloc(int ilen, int capa);
Sequence_t *Sequence_allocptr(void);
Sequence_t *Sequence_allocptrwithcapa(int capa);
Sequence_t *Sequence_allocfromfile(FILE *stream, size_t rs);
Sequence_t *Sequence_clone(Sequence_t *self);
void Sequence_init(Sequence_t *self, int ilen, int capa);
void Sequence_initptr(Sequence_t *self);
void Sequence_copy(Sequence_t *self, Sequence_t *src);
void Sequence_fin(Sequence_t *self);
void Sequence_free(Sequence_t *self);

/* update */
void Sequence_clear(Sequence_t *self);
void Sequence_concat(Sequence_t *self, Sequence_t *v);
void Sequence_insert(Sequence_t *self, int i, void *v);
void Sequence_insertptr(Sequence_t *self, int i, void *v);
void Sequence_insertuchar(Sequence_t *self, int i, uint32_t v);
void Sequence_limit(Sequence_t *self, int n);
void Sequence_offset(Sequence_t *self, int n);
void *Sequence_peekptr(Sequence_t *self);
uint32_t Sequence_peekuchar(Sequence_t *self);
void *Sequence_popptr(Sequence_t *self);
uint32_t Sequence_popuchar(Sequence_t *self);
void Sequence_pushptr(Sequence_t *self, void *v);
void Sequence_pushuchar(Sequence_t *self, uint32_t v);
void Sequence_remove(Sequence_t *self, int i);
void *Sequence_removeptr(Sequence_t *self, int i);
uint32_t Sequence_removeuchar(Sequence_t *self, int i);
void Sequence_resize(Sequence_t *self, int s);
void Sequence_resizeitem(Sequence_t *self, Sequence_t *other);
void Sequence_reverse(Sequence_t *self);
void Sequence_set(Sequence_t *self, int i, void *v);
void Sequence_setptr(Sequence_t *self, int i, void *v);
void Sequence_setrange(Sequence_t *self, int b, int e, Sequence_t *v);
void Sequence_setuchar(Sequence_t *self, int i, uint32_t v);
void *Sequence_shiftptr(Sequence_t *self);
void Sequence_slice(Sequence_t *self, int offset, int limit);
const char *Sequence_tocstr(Sequence_t *self);
void Sequence_unshiftptr(Sequence_t *self, void *v);

/* info */
#define LIST_AT(self, i) ((self)->first + (self)->data->ilen * i)
#define LIST_ATPTR(self, i) (*(void **)LIST_AT(self, (i)))
int Sequence_cmp(const Sequence_t *self, const Sequence_t *other);
int Sequence_cmpcstr(const Sequence_t *self, const char *other);
#define LIST_COUNT(self) ((self)->count)
#define LIST_EACH(self, i, v, block) do { \
    Sequence_t *_l = (self); \
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
    ? 0 : Sequence_cmp((self), (other)) == 0 \
)
#include "cset.h"
int Sequence_findcset(const Sequence_t *self, const CharacterSet_t *pat, int o);
int Sequence_findlist(const Sequence_t *self, const Sequence_t *pat, int o);
int Sequence_finduchar(const Sequence_t *self, uint32_t pat, int o);
void *Sequence_get(const Sequence_t *self, int i);
void *Sequence_getptr(const Sequence_t *self, int i);
uint32_t Sequence_getuchar(const Sequence_t *self, int i);
int Sequence_hc(const Sequence_t *self);
#define LIST_ISINIT(self) ((self)->data != NULL)
void Sequence_write(const Sequence_t *self, FILE *stream);
#endif
