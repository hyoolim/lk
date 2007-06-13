#ifndef PT_SET_H
#define PT_SET_H
#include "_common.h"

/* base set item type; usually bigger to include value */
typedef struct pt_setitem {
    const void *key;
} pt_setitem_t;

/* set data so that cloning isn't so expensive */
typedef int pt_sethashfunc_t(const void *key, int capa);
typedef int pt_setkeycmpfunc_t(const void *self, const void *other);
struct pt_setdata {
    int               ci;
    int               capa;
    int               ivlen; /* length of item value */
    int               refc;
    int               count;
    pt_sethashfunc_t *hashfunc;
    pt_setkeycmpfunc_t  *cmpfunc;
    pt_setitem_t      items; /* placeholder for the first item */
};

/* the actual set */
typedef struct pt_set {
    struct pt_setdata *data;
} pt_set_t;

/* for set construction/destruction */
pt_set_t *pt_set_alloc(int ivlen, pt_sethashfunc_t *hashfunc,
                       pt_setkeycmpfunc_t *cmpfunc);
pt_set_t *pt_set_clone(pt_set_t *self);
void pt_set_copy(pt_set_t *self, pt_set_t *src);
void pt_set_fin(pt_set_t *self);
void pt_set_free(pt_set_t *self);
void pt_set_init(pt_set_t *self, int ivlen, pt_sethashfunc_t *hashfunc,
                 pt_setkeycmpfunc_t *cmpfunc);

/* set manipulation */
void pt_set_clear(pt_set_t *self);
int pt_set_count(pt_set_t *self);
pt_setitem_t *pt_set_get(const pt_set_t *self, const void *key);
void *pt_set_set(pt_set_t *self, const void *key);
void pt_set_unset(pt_set_t *self, const void *key);

/* default hash and keycmp */
int pt_set_keycmp(const void *self, const void *other);
int pt_set_hash(const void *key, int capa);

/* set item pointer math */
#define PT_SETITEM_ADD(data, item, delta) ((pt_setitem_t *)((char *)(item) + PT_SETITEM_SIZE(data) * (delta)))
#define PT_SETITEM_AT(data, index) PT_SETITEM_ADD(data, &(data)->items, index)
#define PT_SETITEM_SIZE(data) (sizeof(pt_setitem_t) + (data)->ivlen)
#define PT_SETITEM_VALUE(type, item) (*(type *)PT_SETITEM_VALUEPTR(item))
#define PT_SETITEM_VALUEPTR(item) ((void *)((char *)(item) + sizeof(pt_setitem_t)))

/* placeholder key for deleted items */
#define PT_SETITEM_SKIPKEY ((pt_setitem_t *)1)

/* simple way to iterate over the whole set */
#define PT_SET_EACH(self, item, block) do { \
    struct pt_setdata *_data = (self)->data; \
    int _i, _isize = PT_SETITEM_SIZE(_data); \
    pt_setitem_t *item = (pt_setitem_t *)&_data->items; \
    item = (pt_setitem_t *)((char *)item - _isize); \
    for(_i = 0; _i < _data->capa; _i ++) { \
        item = (pt_setitem_t *)((char *)item + _isize); \
        if(item->key == NULL || item->key == PT_SETITEM_SKIPKEY) continue; \
        { block; } \
    } \
} while(0)
#endif
