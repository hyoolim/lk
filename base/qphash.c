#include "qphash.h"

/* set cap needs to be primes for quadratic probing to work */
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
    self = mem_alloc(sizeof(struct setdata)
                         - sizeof(setitem_t)
                         + (sizeof(setitem_t) + ivlen) * c);
    self->ci = ci;
    self->cap = c;
    self->ivlen = ivlen;
    self->refc = 1;
    self->size = 0;
    self->hashfunc = hashfunc;
    self->cmpfunc = cmpfunc;
    return self;
}

/* simple ref sizeing mem management for clones */
static void setdata_free(struct setdata *self) {
    if(self->refc > 0) self->refc --;
    if(self->refc < 1) mem_free(self);
}

/* create new set data and repopulate */
static void qphash_resize(qphash_t *self, int ci) {
    struct setdata *olddata = self->data, *newdata;
    sethashfunc_t *hashfunc = olddata->hashfunc;
    setkeycmpfunc_t *cmpfunc = olddata->cmpfunc;
    int delta, newcap;
    setitem_t *newitem, *newlast;
    newdata = setdata_alloc(olddata->ivlen, ci, hashfunc, cmpfunc);
    newlast = SETITEM_AT(newdata, newdata->cap - 1);
    newcap = newdata->cap;
    SET_EACH(self, olditem,
        delta = 1;
        newitem = SETITEM_AT(newdata, hashfunc(olditem->key, newcap));
        while(newitem->key != NULL && (newitem->key == SETITEM_SKIPKEY
        || cmpfunc(newitem->key, olditem->key) != 0)) {
            newitem = SETITEM_ADD(newdata, newitem, delta);
            while(newitem > newlast) {
                newitem = SETITEM_ADD(newdata, newitem, -newcap);
            }
            delta += 2;
        }
        memcpy(newitem, olditem, SETITEM_SIZE(olddata));
    );
    newdata->size = olddata->size;
    setdata_free(olddata);
    self->data = newdata;
}

/* for set construction/deconstruction */
qphash_t *qphash_alloc(int ivlen, sethashfunc_t *hashfunc,
                       setkeycmpfunc_t *cmpfunc) {
    qphash_t *self = mem_alloc(sizeof(qphash_t));
    qphash_init(self, ivlen, hashfunc, cmpfunc);
    return self;
}
qphash_t *qphash_clone(qphash_t *self) {
    qphash_t *clone = mem_alloc(sizeof(qphash_t));
    qphash_copy(clone, self);
    return clone;
}
void qphash_copy(qphash_t *self, qphash_t *src) {
    (self->data = src->data)->refc ++;
}
void qphash_fin(qphash_t *self) {
    setdata_free(self->data);
}
void qphash_free(qphash_t *self) {
    qphash_fin(self);
    mem_free(self);
}
void qphash_init(qphash_t *self, int ivlen, sethashfunc_t *hashfunc,
                 setkeycmpfunc_t *cmpfunc) {
    self->data = setdata_alloc(ivlen, 0, hashfunc, cmpfunc);
}

/* set manipulation */
void qphash_clear(qphash_t *self) {
    struct setdata *data = self->data;
    data->size = 0;
    memset(&data->items, 0x0, SETITEM_SIZE(data) * data->cap);
}
int qphash_size(qphash_t *self) {
    return self->data->size;
}
setitem_t *qphash_get(const qphash_t *self, const void *key) {
    struct setdata *data = self->data;
    int delta = 1, cap = data->cap;
    setitem_t *item = SETITEM_AT(data, data->hashfunc(key, cap));
    setitem_t *last = SETITEM_AT(data, cap - 1);
    while(item->key != NULL) {
        if(item->key != SETITEM_SKIPKEY
        && data->cmpfunc(item->key, key) == 0) {
            return item;
        }
        item = SETITEM_ADD(data, item, delta);
        while(item > last) item = SETITEM_ADD(data, item, -cap);
        delta += 2;
    }
    return NULL;
}
void *qphash_set(qphash_t *self, const void *key) {
    struct setdata *data = self->data;
    int delta = 1, cap;
    setitem_t *item, *last;
    if(data->size >= data->cap / 2) qphash_resize(self, data->ci + 1);
    data = self->data;
    cap = data->cap;
    item = SETITEM_AT(data, data->hashfunc(key, cap));
    last = SETITEM_AT(data, cap - 1);
    while(item->key != NULL && (item->key == SETITEM_SKIPKEY
    || data->cmpfunc(item->key, key) != 0)) {
        item = SETITEM_ADD(data, item, delta);
        while(item > last) item = SETITEM_ADD(data, item, -cap);
        delta += 2;
    }
    if(item->key == NULL || item->key == SETITEM_SKIPKEY) data->size ++;
    item->key = key;
    return SETITEM_VALUEPTR(item);
}
void qphash_unset(qphash_t *self, const void *key) {
    setitem_t *item = qphash_get(self, key);
    if(item != NULL) {
        item->key = SETITEM_SKIPKEY;
        self->data->size --;
    }
}

/* default hash and keycmp */
int qphash_hash(const void *key, int cap) {
    return (ptrdiff_t)key % cap;
}
int qphash_keycmp(const void *self, const void *other) {
    return (ptrdiff_t)self - (ptrdiff_t)other;
}
