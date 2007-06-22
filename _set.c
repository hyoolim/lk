#include "_set.h"

/* set capa needs to be primes for quadratic probing to work */
static unsigned long primes[] = { 
    11, 19, 37, 67, 131, 283, 521, 1033, 2053, 4099, 8219, 16427,
    32771, 65581, 131101, 262147, 524309, 1048583, 2097169, 4194319,
    8388617, 16777259, 33554467, 67108879, 134217757, 268435459,
    536870923, 1073741909, 0
};

/* create set data depending on item size */
static struct setdata *setdata_alloc(int ivlen, int ci,
                                        sethashfunc_t *hashfunc,
                                        setkeycmpfunc_t *cmpfunc) {
    struct setdata *self;
    int c = primes[ci];
    self = memory_alloc(sizeof(struct setdata)
                         - sizeof(setitem_t)
                         + (sizeof(setitem_t) + ivlen) * c);
    self->ci = ci;
    self->capa = c;
    self->ivlen = ivlen;
    self->refc = 1;
    self->count = 0;
    self->hashfunc = hashfunc;
    self->cmpfunc = cmpfunc;
    return self;
}

/* simple ref counting memory management for clones */
static void setdata_free(struct setdata *self) {
    if(self->refc > 0) self->refc --;
    if(self->refc < 1) memory_free(self);
}

/* create new set data and repopulate */
static void set_resize(set_t *self, int ci) {
    struct setdata *olddata = self->data, *newdata;
    sethashfunc_t *hashfunc = olddata->hashfunc;
    setkeycmpfunc_t *cmpfunc = olddata->cmpfunc;
    int delta, newcapa;
    setitem_t *newitem, *newlast;
    newdata = setdata_alloc(olddata->ivlen, ci, hashfunc, cmpfunc);
    newlast = SETITEM_AT(newdata, newdata->capa - 1);
    newcapa = newdata->capa;
    SET_EACH(self, olditem,
        delta = 1;
        newitem = SETITEM_AT(newdata, hashfunc(olditem->key, newcapa));
        while(newitem->key != NULL && (newitem->key == SETITEM_SKIPKEY
        || cmpfunc(newitem->key, olditem->key) != 0)) {
            newitem = SETITEM_ADD(newdata, newitem, delta);
            while(newitem > newlast) {
                newitem = SETITEM_ADD(newdata, newitem, -newcapa);
            }
            delta += 2;
        }
        memcpy(newitem, olditem, SETITEM_SIZE(olddata));
    );
    newdata->count = olddata->count;
    setdata_free(olddata);
    self->data = newdata;
}

/* for set construction/deconstruction */
set_t *set_alloc(int ivlen, sethashfunc_t *hashfunc,
                       setkeycmpfunc_t *cmpfunc) {
    set_t *self = memory_alloc(sizeof(set_t));
    set_init(self, ivlen, hashfunc, cmpfunc);
    return self;
}
set_t *set_clone(set_t *self) {
    set_t *clone = memory_alloc(sizeof(set_t));
    set_copy(clone, self);
    return clone;
}
void set_copy(set_t *self, set_t *src) {
    (self->data = src->data)->refc ++;
}
void set_fin(set_t *self) {
    setdata_free(self->data);
}
void set_free(set_t *self) {
    set_fin(self);
    memory_free(self);
}
void set_init(set_t *self, int ivlen, sethashfunc_t *hashfunc,
                 setkeycmpfunc_t *cmpfunc) {
    self->data = setdata_alloc(ivlen, 0, hashfunc, cmpfunc);
}

/* set manipulation */
void set_clear(set_t *self) {
    struct setdata *data = self->data;
    data->count = 0;
    memset(&data->items, 0x0, SETITEM_SIZE(data) * data->capa);
}
int set_count(set_t *self) {
    return self->data->count;
}
setitem_t *set_get(const set_t *self, const void *key) {
    struct setdata *data = self->data;
    int delta = 1, capa = data->capa;
    setitem_t *item = SETITEM_AT(data, data->hashfunc(key, capa));
    setitem_t *last = SETITEM_AT(data, capa - 1);
    while(item->key != NULL) {
        if(item->key != SETITEM_SKIPKEY
        && data->cmpfunc(item->key, key) == 0) {
            return item;
        }
        item = SETITEM_ADD(data, item, delta);
        while(item > last) item = SETITEM_ADD(data, item, -capa);
        delta += 2;
    }
    return NULL;
}
void *set_set(set_t *self, const void *key) {
    struct setdata *data = self->data;
    int delta = 1, capa;
    setitem_t *item, *last;
    if(data->count >= data->capa / 2) set_resize(self, data->ci + 1);
    data = self->data;
    capa = data->capa;
    item = SETITEM_AT(data, data->hashfunc(key, capa));
    last = SETITEM_AT(data, capa - 1);
    while(item->key != NULL && (item->key == SETITEM_SKIPKEY
    || data->cmpfunc(item->key, key) != 0)) {
        item = SETITEM_ADD(data, item, delta);
        while(item > last) item = SETITEM_ADD(data, item, -capa);
        delta += 2;
    }
    if(item->key == NULL || item->key == SETITEM_SKIPKEY) data->count ++;
    item->key = key;
    return SETITEM_VALUEPTR(item);
}
void set_unset(set_t *self, const void *key) {
    setitem_t *item = set_get(self, key);
    if(item != NULL) {
        item->key = SETITEM_SKIPKEY;
        self->data->count --;
    }
}

/* default hash and keycmp */
int set_hash(const void *key, int capa) {
    return (ptrdiff_t)key % capa;
}
int set_keycmp(const void *self, const void *other) {
    return (ptrdiff_t)self - (ptrdiff_t)other;
}
