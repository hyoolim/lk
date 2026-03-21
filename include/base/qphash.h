#ifndef SET_H
#define SET_H
#include "common.h"

/* base set item type; usually bigger to include value */
typedef struct setitem {
    const void *key;
} setitem_t;

/* set data so that cloning isn't so expensive */
typedef int sethashfunc_t(const void *key, int cap);
typedef int setkeycmpfunc_t(const void *self, const void *other);
struct setdata {
    int               ci;
    int               cap;
    int               ivlen; /* length of item value */
    int               refc;
    int               size;
    sethashfunc_t *hashfunc;
    setkeycmpfunc_t  *cmpfunc;
    setitem_t      items; /* placeholder for the first item */
};

/* the actual set */
typedef struct set {
    struct setdata *data;
} qphash_t;

/* for set construction/destruction */
qphash_t *qphash_alloc(int ivlen, sethashfunc_t *hashfunc,
                       setkeycmpfunc_t *cmpfunc);
qphash_t *qphash_clone(qphash_t *self);
void qphash_copy(qphash_t *self, qphash_t *src);
void qphash_fin(qphash_t *self);
void qphash_free(qphash_t *self);
void qphash_init(qphash_t *self, int ivlen, sethashfunc_t *hashfunc,
                 setkeycmpfunc_t *cmpfunc);

/* set manipulation */
void qphash_clear(qphash_t *self);
int qphash_size(qphash_t *self);
setitem_t *qphash_get(const qphash_t *self, const void *key);
void *qphash_set(qphash_t *self, const void *key);
void qphash_unset(qphash_t *self, const void *key);

/* default hash and keycmp */
int qphash_keycmp(const void *self, const void *other);
int qphash_hash(const void *key, int cap);

/* set item pointer math */
#define SETITEM_ADD(data, item, delta) ((setitem_t *)((char *)(item) + SETITEM_SIZE(data) * (delta)))
#define SETITEM_AT(data, index) SETITEM_ADD(data, &(data)->items, index)
#define SETITEM_SIZE(data) (sizeof(setitem_t) + (data)->ivlen)
#define SETITEM_VALUE(type, item) (*(type *)SETITEM_VALUEPTR(item))
#define SETITEM_VALUEPTR(item) ((void *)((char *)(item) + sizeof(setitem_t)))

/* placeholder key for deleted items */
#define SETITEM_SKIPKEY ((setitem_t *)1)

/* simple way to iterate over the whole set */
#define SET_EACH(self, item, block) do { \
    struct setdata *_data = (self)->data; \
    int _i, _isize = SETITEM_SIZE(_data); \
    setitem_t *item = (setitem_t *)&_data->items; \
    item = (setitem_t *)((char *)item - _isize); \
    for(_i = 0; _i < _data->cap; _i ++) { \
        item = (setitem_t *)((char *)item + _isize); \
        if(item->key == NULL || item->key == SETITEM_SKIPKEY) continue; \
        { block; } \
    } \
} while(0)
#endif
