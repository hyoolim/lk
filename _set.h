#ifndef SET_H
#define SET_H
#include "_common.h"

/* base set item type; usually bigger to include value */
typedef struct setitem {
    const void *key;
} setitem_t;

/* set data so that cloning isn't so expensive */
typedef int sethashfunc_t(const void *key, int capa);
typedef int setkeycmpfunc_t(const void *self, const void *other);
struct setdata {
    int               ci;
    int               capa;
    int               ivlen; /* length of item value */
    int               refc;
    int               count;
    sethashfunc_t *hashfunc;
    setkeycmpfunc_t  *cmpfunc;
    setitem_t      items; /* placeholder for the first item */
};

/* the actual set */
typedef struct set {
    struct setdata *data;
} set_t;

/* for set construction/destruction */
set_t *set_alloc(int ivlen, sethashfunc_t *hashfunc,
                       setkeycmpfunc_t *cmpfunc);
set_t *set_clone(set_t *self);
void set_copy(set_t *self, set_t *src);
void set_fin(set_t *self);
void set_free(set_t *self);
void set_init(set_t *self, int ivlen, sethashfunc_t *hashfunc,
                 setkeycmpfunc_t *cmpfunc);

/* set manipulation */
void set_clear(set_t *self);
int set_count(set_t *self);
setitem_t *set_get(const set_t *self, const void *key);
void *set_set(set_t *self, const void *key);
void set_unset(set_t *self, const void *key);

/* default hash and keycmp */
int set_keycmp(const void *self, const void *other);
int set_hash(const void *key, int capa);

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
    for(_i = 0; _i < _data->capa; _i ++) { \
        item = (setitem_t *)((char *)item + _isize); \
        if(item->key == NULL || item->key == SETITEM_SKIPKEY) continue; \
        { block; } \
    } \
} while(0)
#endif
